#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_map>

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    const Cell* GetCommonCell(Position pos) const;
    Cell* GetCommonCell(Position pos);

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
    struct Row {
        std::vector<std::unique_ptr<Cell>> cells{};
    };

    struct SheetData {
        std::vector<std::unique_ptr<Row>> rows{};
    };

    SheetData data_sheet;
};