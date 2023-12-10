#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

//std::ostream& operator<<(std::ostream& output, FormulaError fe) {
//    return output << "#ARITHM!";
//}

namespace {
    class Formula : public FormulaInterface {
    public:
// Реализуйте следующие методы:

    explicit Formula(std::string expression)
    try : ast_(ParseFormulaAST(expression)) {
    } catch (const std::exception& exc) {
        std::throw_with_nested(FormulaException(exc.what()));
    }

    Value Evaluate(const SheetInterface& sheet) const override {
        try {
            Value val = ast_.Execute(sheet);
            return val;
        } catch (const FormulaError& error) {
            return error;
        }
    }

    std::string GetExpression() const override {
        std::stringstream str_out;

        ast_.PrintFormula(str_out);
        return str_out.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        return {ast_.GetCells().begin(), ast_.GetCells().end()};
    }

    private:
        FormulaAST ast_;
//        mutable std::optional<Value> cache_;
    };
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}