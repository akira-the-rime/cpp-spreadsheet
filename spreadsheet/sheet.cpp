#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

#include "sheet.h"

namespace detail {
    struct Visitor final {
        void operator()(std::ostream& os, const std::string& text) {
            os << text;
        }

        void operator()(std::ostream& os, double number) {
            os << number;
        }

        void operator()(std::ostream& os, FormulaError fe) {
            os << fe;
        }
    };
} // namespace detail

Sheet::~Sheet() noexcept = default;

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }

    if (spreadsheet_.count(pos) && spreadsheet_.at(pos) != nullptr) {
        spreadsheet_.at(pos)->Clear();

        if (spreadsheet_.at(pos)->HasUpperLevel()) {
            return;
        }

        spreadsheet_.at(pos).reset(nullptr);
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }

    if (!spreadsheet_.count(pos)) {
        return nullptr;
    }

    return spreadsheet_.at(pos).get();
}

CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }

    if (!spreadsheet_.count(pos)) {
        return nullptr;
    }

    return spreadsheet_.at(pos).get();
}

Size Sheet::GetPrintableSize() const noexcept {
    int row_size = 0;
    int col_size = 0;

    if (spreadsheet_.empty()) {
        return { row_size, col_size };
    }

    for (const auto& [pos, cell] : spreadsheet_) {
        if (cell != nullptr) {
            if (row_size <= pos.row) {
                row_size = pos.row + 1;
            }
            
            if (col_size <= pos.col) {
                col_size = pos.col + 1;
            }
        }
    }

    return { row_size, col_size };
}

void Sheet::PrintTexts(std::ostream& output) const noexcept {
    Size size = GetPrintableSize();

    for (int i = 0; i < size.rows; ++i) {
        for (int j = 0; j < size.cols; ++j) {
            Position pos = { i, j };

            if (spreadsheet_.count(pos) && spreadsheet_.at(pos) != nullptr) {
                output << spreadsheet_.at(pos)->GetText();
            }

            if (j != size.cols - 1) {
                output << '\t';
            }
        }

        output << '\n';
    }
}

void Sheet::PrintValues(std::ostream& output) const {
    Size size = GetPrintableSize();

    for (int i = 0; i < size.rows; ++i) {
        for (int j = 0; j < size.cols; ++j) {
            Position pos = { i, j };

            if (spreadsheet_.count(pos) && spreadsheet_.at(pos) != nullptr) {
                detail::Visitor visitor;

                std::visit([&visitor, &output](auto&& value) {
                    visitor(output, value);
                },
                    spreadsheet_.at(pos)->GetValue());
            }

            if (j != size.cols - 1) {
                output << '\t';
            }
        }

        output << '\n';
    }
}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }

    if (!spreadsheet_.count(pos)) {
        spreadsheet_.emplace(pos, std::make_unique<Cell>(*this));
    }

    spreadsheet_.at(pos)->Set(std::move(text));
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}