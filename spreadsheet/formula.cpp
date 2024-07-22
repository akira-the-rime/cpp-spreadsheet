#include <algorithm>
#include <cassert>
#include <cctype>
#include <iterator>
#include <sstream>

#include "formula.h"
#include "FormulaAST.h"

FormulaError::FormulaError(Category category) 
    : category_(category) {
}

bool FormulaError::operator==(FormulaError rhs) const noexcept {
    return this->category_ == rhs.category_;
}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

std::string_view FormulaError::ToString() const {
    switch (category_) {
    case Category::Ref:
        return "#REF!";
    case Category::Value:
        return "#VALUE!";
    case Category::Arithmetic:
        return "#ARITHM!";
    }

    return {};
}

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

namespace {
    class Formula final : public FormulaInterface {
    public:
        explicit Formula(std::string expression) try 
            : ast_(ParseFormulaAST(expression)) {
        }
        catch (const std::exception& exc) {
            throw FormulaException("Invalid formula.");
        }

        Value Evaluate(const SheetInterface& spreadsheet) const override {
            try {
                return ast_.Execute(spreadsheet);
            }
            catch (const FormulaError& exc) {
                return exc;
            }
        }

        std::string GetExpression() const override {
            std::stringstream ss;
            ast_.PrintFormula(ss);

            return ss.str();
        }

        std::vector<Position> GetReferencedCells() const override {
            std::vector<Position> unique_cells;
            
            for (auto& cell : ast_.GetCells()) {
                if (cell.IsValid()) {
                    unique_cells.push_back(cell);
                }
            }

            auto to_delete_begin = std::unique(unique_cells.begin(), unique_cells.end());
            unique_cells.erase(to_delete_begin, unique_cells.end());

            return unique_cells;
        }

    private:
        FormulaAST ast_;
    };
} // unnamed namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}