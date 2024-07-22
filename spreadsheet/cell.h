#pragma once

#include <optional>
#include <unordered_set>

#include "common.h"
#include "formula.h"
#include "sheet.h"

namespace detail {
    class Impl {
    public:
        using Value = std::variant<std::string, double, FormulaError>;

        virtual ~Impl() noexcept = default;

        virtual std::vector<Position> GetReferencedCells() const = 0;
        virtual std::string GetText() const noexcept = 0;
        virtual Value GetValue() const = 0;
    };

    class EmptyImpl final : public Impl {
    public:
        EmptyImpl(std::string text)
            : text_(text) {
        }

        std::vector<Position> GetReferencedCells() const override {
            return {};
        }

        std::string GetText() const noexcept override {
            return text_;
        }

        Value GetValue() const override {
            if (text_.length() == 0) {
                return 0.0;
            }
            else if (text_.front() == ESCAPE_SIGN) {
                return "";
            }

            return text_;
        }

    private:
        std::string text_;
    };

    class TextImpl final : public Impl {
    public:
        TextImpl(std::string text)
            : text_(text) {
        }

        std::vector<Position> GetReferencedCells() const override {
            return {};
        }

        std::string GetText() const noexcept override {
            return text_;
        }

        Value GetValue() const override {
            if (text_.front() == ESCAPE_SIGN) {
                return text_.substr(1, text_.size() - 1);
            }

            return text_;
        }

    private:
        std::string text_;
    };

    class FormulaImpl final : public Impl {
    public:
        FormulaImpl(std::string text, const SheetInterface& spreadsheet)
            : formula_(ParseFormula(text.substr(1, text.size() - 1))) 
            , spreadsheet_(spreadsheet) {
        }

        std::vector<Position> GetReferencedCells() const override {
            return formula_->GetReferencedCells();
        }

        std::string GetText() const noexcept override {
            return "=" + formula_->GetExpression();
        }

        Value GetValue() const override {
            if (!cache_.has_value()) {
                cache_ = formula_->Evaluate(spreadsheet_);
            }

            if (std::holds_alternative<double>(cache_.value())) {
                return std::get<double>(cache_.value());
            }

            return std::get<FormulaError>(cache_.value());
        }

        void InvalidateCache() noexcept {
            cache_.reset();
        }

    private:
        std::unique_ptr<FormulaInterface> formula_;
        const SheetInterface& spreadsheet_;

        mutable std::optional<std::variant<double, FormulaError>> cache_;
    };
} // namespace detail

class Sheet;

class Cell final : public CellInterface {
public:
    Cell(Sheet& spreadsheet);
    ~Cell() noexcept;

    void Clear() noexcept;
    std::vector<Position> GetReferencedCells() const override;
    std::string GetText() const noexcept override;
    Value GetValue() const override;
    bool HasUpperLevel() const;
    void Set(std::string text);

private:
    void AdjustCellsDependency(const detail::Impl* const being_considered_impl);
    bool CheckOnCyclicDependency(const detail::Impl* const being_considered_impl);
    void InvalidateCache(const std::unordered_set<Cell*>& to_invalidate);

    std::unique_ptr<detail::Impl> impl_;
    Sheet& spreadsheet_;

    std::unordered_set<Cell*> upper_level_;
    std::unordered_set<Cell*> lower_level_;
};