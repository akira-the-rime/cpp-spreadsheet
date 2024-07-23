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

void Sheet::CheckPositionValidity(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }
}

void Sheet::ClearCell(Position pos) {
    CheckPositionValidity(pos);
    auto taken_cell = spreadsheet_.find(pos);

    if (taken_cell != spreadsheet_.end() && taken_cell->second != nullptr) {
        taken_cell->second->Clear();

        if (taken_cell->second->HasUpperLevel()) {
            return;
        }

        taken_cell->second.reset(nullptr);
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    CheckPositionValidity(pos);
    auto taken_cell = spreadsheet_.find(pos);

    if (taken_cell == spreadsheet_.end()) {
        return nullptr;
    }

    return taken_cell->second.get();
}

CellInterface* Sheet::GetCell(Position pos) {
    CheckPositionValidity(pos);
    auto taken_cell = spreadsheet_.find(pos);

    if (taken_cell == spreadsheet_.end()) {
        return nullptr;
    }

    return taken_cell->second.get();
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

            auto taken_cell = spreadsheet_.find(pos);

            if (taken_cell != spreadsheet_.end() && taken_cell->second != nullptr) {
                output << taken_cell->second->GetText();
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

            auto taken_cell = spreadsheet_.find(pos);

            if (taken_cell != spreadsheet_.end() && taken_cell->second != nullptr) {
                detail::Visitor visitor;

                std::visit([&output, &taken_cell, &visitor](auto&& value) {
                    visitor(output, value);
                },
                    taken_cell->second->GetValue());
            }

            if (j != size.cols - 1) {
                output << '\t';
            }
        }

        output << '\n';
    }
}

void Sheet::SetCell(Position pos, std::string text) {
    CheckPositionValidity(pos);

    if (!spreadsheet_.count(pos)) {
        spreadsheet_.emplace(pos, std::make_unique<Cell>(*this));
    }

    spreadsheet_.at(pos)->Set(std::move(text));
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}