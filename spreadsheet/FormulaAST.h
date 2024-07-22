#pragma once

#include <forward_list>
#include <stdexcept>

#include "common.h"
#include "FormulaLexer.h"

namespace ASTImpl {
    class Expr;
} // namespace ASTImpl

class ParsingError final : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class FormulaAST final {
public:
    explicit FormulaAST(std::unique_ptr<ASTImpl::Expr> root_expr, std::forward_list<Position> cells);
    FormulaAST(FormulaAST&&) noexcept = default;
    FormulaAST& operator=(FormulaAST&&) noexcept = default;
    ~FormulaAST() noexcept;

    double Execute(const SheetInterface& spreadsheet) const;
    const std::forward_list<Position>& GetCells() const noexcept;
    std::forward_list<Position>& GetCells() noexcept;
    void Print(std::ostream& out) const;
    void PrintFormula(std::ostream& out) const;

private:
    std::unique_ptr<ASTImpl::Expr> root_expr_;
    std::forward_list<Position> referenced_cells_;
};

FormulaAST ParseFormulaAST(std::istream& in);
FormulaAST ParseFormulaAST(const std::string& in_str);