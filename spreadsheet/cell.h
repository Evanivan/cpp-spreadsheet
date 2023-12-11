#pragma once

#include <optional>
#include <unordered_set>
#include <algorithm>

#include "common.h"
#include "formula.h"

class Cell : public CellInterface {
public:
    Cell(SheetInterface& table, Position pos);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool HasCache() const {
        return cached_value_.has_value();
    }
    void ClearCache() {
        cached_value_.reset();
    }

private:
    SheetInterface& table_;
    Position pos_;
    std::unique_ptr<FormulaInterface> val_;
    std::optional<std::string> text_;

    std::vector<Position> referenced_cells_;
    std::vector<Position> referring_cells_;
    mutable std::optional<double> cached_value_;

    void CacheInvalidation(const std::vector<Position>& referring_cells);
    void HasCircularDependency(const Position& current_pos, const std::vector<Position>& references,
                                  std::unordered_set<Position, PositionHasher>& visited_cells) const;
};