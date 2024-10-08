#pragma once

#include <memory>
#include <vector>

#include "common.h"

class FormulaInterface {
public:
    using Value = std::variant<double, FormulaError>;

    virtual ~FormulaInterface() noexcept = default;
    virtual Value Evaluate(const SheetInterface& spreadsheet) const = 0;
    virtual std::string GetExpression() const = 0;
    virtual std::vector<Position> GetReferencedCells() const = 0;
};

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression);