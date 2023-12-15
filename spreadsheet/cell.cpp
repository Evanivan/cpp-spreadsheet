#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

#include "sheet.h"

// Реализуйте следующие методы
using namespace std::literals;

Cell::Cell(Sheet& table, Position pos)
        : table_(table)
        , pos_(pos){
    text_ = "";
    val_ = nullptr;

}

Cell::~Cell() {
    Cell::Clear();
}

void Cell::Set(std::string text) {
    std::unique_ptr<FormulaInterface> tmp_formula_ptr = nullptr;
    std::vector<Position> tmp_referenced_cells{};

    if (text[0] == FORMULA_SIGN && text.size() > 1) { //expression
        tmp_formula_ptr = ParseFormula({text.begin() + 1, text.end()});

        tmp_referenced_cells = tmp_formula_ptr->GetReferencedCells();
        std::unordered_set<Position, PositionHasher> visited_cells{};
        HasCircularDependency(pos_, tmp_referenced_cells, visited_cells);
    }
    //Erasing all current references
    for (const Position& cell_pos : referenced_cells_) {
        Cell* cell = table_.GetCommonCell(cell_pos);
        if (cell) {
            // Find the position in referring_cells_ and erase it
            auto it = std::find(cell->referring_cells_.begin(), cell->referring_cells_.end(), pos_);
            if (it != cell->referring_cells_.end()) {
                cell->referring_cells_.erase(it);
            }
        }
    }
    referenced_cells_.clear();

    if (tmp_formula_ptr) {
        for (const Position &pos: tmp_referenced_cells) {
            if (table_.GetCell(pos) == nullptr) {
                table_.SetCell(pos, ""s);
            }
            referenced_cells_.emplace_back(pos);

            Cell *ref_cell_no_const = table_.GetCommonCell(pos);
            ref_cell_no_const->referring_cells_.emplace_back(pos_);
        }
        val_ = std::move(tmp_formula_ptr);
    }

    if (!text.empty()) {
        text_ = std::move(text);
    } else {
        text_ = "";
        val_ = nullptr;
    }
    CacheInvalidation();
}

void Cell::Clear() {
    Set("");
}

Cell::Value Cell::GetValue() const {
    if (text_.has_value() && val_ == nullptr) {
        if (text_.value().empty()) {
            return std::string("");
        } else if (text_.value()[0] == ESCAPE_SIGN) {
            return std::string{text_->begin() + 1, text_->end()};
        }
    }

    if (cached_value_.has_value()) {
        return cached_value_.value();
    }

    if (val_ != nullptr) {
        try {
            FormulaInterface::Value result = val_->Evaluate(table_);
            if (std::holds_alternative<double>(result)) {
                cached_value_ = std::get<double>(result);
                return cached_value_.value();
            } else if (std::holds_alternative<FormulaError>(result)) {
                return std::get<FormulaError>(result);
            }
        } catch (const FormulaException &e) {
            return FormulaError(e.what());
        }
    }
    return text_.value_or("");  // return text if no value or an empty value
}

std::string Cell::GetText() const {
    if (val_ != nullptr) {
        return FORMULA_SIGN + val_->GetExpression();
    }
    return text_.value();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return referenced_cells_;
}

bool Cell::HasCache() const {
    return cached_value_.has_value();
}
void Cell::ClearCache() {
    cached_value_.reset();
}

void Cell::CacheInvalidation() {
    ClearCache();
    for (const Position& cell_pos : referring_cells_) {
        Cell* cell = table_.GetCommonCell(cell_pos);

        if (!cell->HasCache()) {
            continue;
        } else {
            cell->ClearCache();
            CacheInvalidation();
        }
    }
}

void Cell::HasCircularDependency(const Position& current_pos, const std::vector<Position>& references,
                                 std::unordered_set<Position, PositionHasher>& visited) const {
    for (const Position& ref_pos : references) {
        if (visited.count(ref_pos)) continue;
        if (current_pos == ref_pos) throw CircularDependencyException("Circular dependency");
        visited.emplace(ref_pos);

        auto ref_cell = table_.GetCell(ref_pos);
        if (ref_cell) {
            HasCircularDependency(current_pos, ref_cell->GetReferencedCells(), visited);
        }
    }
}

std::vector<Position> Cell::GetCellReferring() const {
    return referring_cells_;
}
