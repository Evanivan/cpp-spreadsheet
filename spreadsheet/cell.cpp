#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


// Реализуйте следующие методы
using namespace std::literals;

Cell::Cell(const SheetInterface &table, Position pos)
        : table_(table)
          , pos_(pos){
    text_ = "";
    val_ = nullptr;

}

Cell::~Cell() {
    Cell::Clear();
}

void Cell::Set(std::string text) {
    for (const Position& cell_pos : referenced_cells_) {
        const CellInterface* cell_interface = table_.GetCell(cell_pos);
        if (cell_interface) {
            Cell* cell = const_cast<Cell*>(dynamic_cast<const Cell*>(cell_interface));

            // Find the position in referring_cells_ and erase it
            auto it = std::find(cell->referring_cells_.begin(), cell->referring_cells_.end(), pos_);
            if (it != cell->referring_cells_.end()) {
                cell->referring_cells_.erase(it);
            }
        }
    }
    referenced_cells_.clear();


    if (!text.empty()) {
        if (text[0] == FORMULA_SIGN && text.size() > 1) { //expression
            std::unique_ptr<FormulaInterface> formula_ptr = ParseFormula({text.begin() + 1, text.end()});
            auto formula_result = formula_ptr->Evaluate(table_);

//            if (std::holds_alternative<FormulaError>(formula_result)) {
//                throw FormulaException("#ARITHM!");
//            } else {
                const auto& refs = formula_ptr->GetReferencedCells();
                std::unordered_set<Position, PositionHasher> visited_cells{};
                HasCircularDependency(pos_, refs, visited_cells);

                for (const Position& pos : refs) {
                    if (table_.GetCell(pos) == nullptr) {
                        auto& non_const_sheet = const_cast<SheetInterface&>(table_);
                        non_const_sheet.SetCell(pos, ""s);
                    }
                    referenced_cells_.emplace_back(pos);

                    const CellInterface* ref_interface_const = table_.GetCell(pos);
                    Cell* ref_cell_no_const = const_cast<Cell*>(dynamic_cast<const Cell*>(ref_interface_const));
                    ref_cell_no_const->referring_cells_.emplace_back(pos_);
                }
                if (std::holds_alternative<FormulaError>(formula_result)) {
                    val_ = std::move(formula_ptr);
                    text_.value() = text;
//                    throw FormulaException("#ARITHM!");
                } else {
                    val_ = std::move(formula_ptr);
                    text_.value() = text;
                    cached_value_ = std::get<double>(formula_result);
                }
//            }
        } else {
            text_.value() = text;
        }
    } else {
        text_ = "";
        text_ = nullptr;
    }
}

void Cell::Clear() {
    text_.reset();
    val_.release();
}

Cell::Value Cell::GetValue() const {
    if (text_.has_value() && val_ == nullptr) {
        if (text_.value().empty()) {
            return std::string("");
        } else if (text_.value()[0] == ESCAPE_SIGN) {
            return std::string{text_->begin() + 1, text_->end()};
        }
    }

    if (val_ != nullptr) {
        try {
            FormulaInterface::Value result = val_->Evaluate(table_);
            if (std::holds_alternative<double>(result)) {
                return std::get<double>(result);
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
        return '=' + val_->GetExpression();
    }
    return text_.value();
}

std::vector<Position> Cell::GetReferencedCells() const {
    if (!val_) {
        return {};
    }

    std::vector<Position> referenced_cells = val_->GetReferencedCells();

    std::sort(referenced_cells.begin(), referenced_cells.end());
    auto last = std::unique(referenced_cells.begin(), referenced_cells.end());
    referenced_cells.erase(last, referenced_cells.end());

    return referenced_cells;

}

void Cell::CacheInvalidation(const std::vector<Position>& referring_cells) {
    for (const Position& cell_pos : referring_cells) {
        const Cell* cell = dynamic_cast<const Cell*>(table_.GetCell(cell_pos));
        Cell* cell_no_const = const_cast<Cell*>(cell);

        if (!cell_no_const->HasCache()) {
            continue;
        } else {
            cell_no_const->ClearCache();
            CacheInvalidation(cell_no_const->referenced_cells_);
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
