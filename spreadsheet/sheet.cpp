#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <string>

using namespace std::literals;

Sheet::~Sheet() {
    for (auto& row : data_sheet.rows) {
        row->cells.clear();
    }
    data_sheet.rows.clear();
}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }

    // Expand the rows to reach the required position.
    while (pos.row >= static_cast<int>(data_sheet.rows.size())) {
        data_sheet.rows.push_back(std::make_unique<Row>());
    }

    auto& row = data_sheet.rows[pos.row];

    // Expand the cells within the row to reach the required position.
    // Also, initialize new empty cells as needed.
    while (pos.col >= static_cast<int>(row->cells.size())) {
        Position cellPos(pos.row, row->cells.size());  // Create position for the new cell.
        row->cells.push_back(std::make_unique<Cell>(*this, cellPos));
    }

    // Now that we have ensured that the cell exists, we can safely set its text.
    row->cells[pos.col]->Set(text);
}


//void Sheet::SetCell(Position pos, std::string text) {
//    if (!pos.IsValid()) {
//        throw InvalidPositionException("Invalid position");
//    }
//
//    while (pos.row >= static_cast<int>(data_sheet.rows.size())) {
//        data_sheet.rows.push_back(std::make_unique<Row>());
//    }
//
//    auto& row = data_sheet.rows[pos.row];
//    while (pos.col >= static_cast<int>(row->cells.size())) {
//        row->cells.push_back(std::make_unique<Cell>());
//    }
//
//    row->cells[pos.col]->Set(text);
//}

//void Sheet::SetCell(Position pos, std::string text) {
//
//    if (auto p = GetCell(pos)) {
//        if (dynamic_cast<const Cell*>(p)->GetText() == text) {
//            return;
//        }
//
//        cells_[pos.row][pos.col]->Set(text);
//        return;
//    }
//
//    std::unique_ptr<Cell> cell = std::make_unique<Cell>(*this, pos);
//    cell->Set(text);
//
//    ResizeTable(pos);
//    cells_[pos.row][pos.col] = std::move(cell);
//
//    if (!text.empty()) {
//        SetSize(pos);
//    }
//}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }
    if (int(data_sheet.rows.size()) <= pos.row) {
        return nullptr;
    }
    if (int(data_sheet.rows.at(pos.row)->cells.size()) <= pos.col) {
        return nullptr;
    }
    if (data_sheet.rows[pos.row]->cells[pos.col] != nullptr) {
        return data_sheet.rows[pos.row]->cells[pos.col].get();
    }
    return nullptr;
}
CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }
    if (int(data_sheet.rows.size()) <= pos.row) {
        return nullptr;
    }
    if (int(data_sheet.rows.at(pos.row)->cells.size()) <= pos.col) {
        return nullptr;
    }
    if (data_sheet.rows[pos.row]->cells[pos.col] != nullptr) {
        return data_sheet.rows[pos.row]->cells[pos.col].get();
    }
    return nullptr;
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid pos");
    }
    if (int(data_sheet.rows.size()) <= pos.row) {
        return;
    }
    if (int(data_sheet.rows.at(pos.row)->cells.size()) <= pos.col) {
        return;
    }
    if (pos.row >= static_cast<int>(data_sheet.rows.size())
        || pos.col >= static_cast<int>(data_sheet.rows[pos.row]->cells.size())) {
        return;
    }
    data_sheet.rows[pos.row]->cells.at(pos.col).reset();
}

Size Sheet::GetPrintableSize() const {
    int max_row = 0;
    int max_col = 0;

    int i = 1;
    for (const auto& row : data_sheet.rows) {
        int j = 1;
        for (const auto& cell : row->cells) {
            if (cell != nullptr && !cell->GetText().empty()) {
                if (j > max_col) {
                    max_col = j;
                }
                if (i > max_row) {
                    max_row = i;
                }
            }
            ++j;
        }
        ++i;
    }

    return {max_row, max_col};
}

void Sheet::PrintValues(std::ostream& output) const {
    auto table_size = GetPrintableSize();

    for (int i = 0; i < table_size.rows; ++i) {
        for (int j = 0; j < table_size.cols; ++j) {
            if (const auto& row = data_sheet.rows[i]; j < int(row->cells.size())) {
                const auto& cell = row->cells[j];
                if (cell) {
                    const auto result = cell->GetValue();
                    if (std::holds_alternative<double>(result)) {
                        output << std::get<double>(result);
                    } else if (std::holds_alternative<FormulaError>(result)) {
                        output << std::get<FormulaError>(result);
                    } else if (!cell->GetText().empty()){
                        std::string tmp_text = cell->GetText();
                        if (*tmp_text.begin() == ESCAPE_SIGN) {
                            output << std::string(tmp_text.begin() + 1, tmp_text.end());
                        } else {
                            output << cell->GetText();
                        }
                    }
                }
            }
            if (j + 1 < table_size.cols) {
                output << '\t';
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    auto table_size = this->GetPrintableSize();

    for (int i = 0; i < table_size.rows; ++i) {
        for (int j = 0; j < table_size.cols; ++j) {
            if (const auto& row = data_sheet.rows[i]; j < int(row->cells.size())) {
                const auto& cell = row->cells[j];
                if (cell) {
                    output << cell->GetText();
                }
            }
            if (j + 1 < table_size.cols) {
                output << '\t';
            }
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}