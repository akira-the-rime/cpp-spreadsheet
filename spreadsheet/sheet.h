#pragma once

#include <cstddef>
#include <unordered_map>

#include "cell.h"
#include "common.h"

struct PosComparator final {
    using is_transparent = std::false_type;

    bool operator()(Position lhs, Position rhs) const {
        return lhs == rhs;
    }
};

struct PosHasher final {
    std::size_t operator()(Position pos) const {
        return string_hasher_(pos.ToString());
    }

private:
    std::hash<std::string> string_hasher_;
};

class Cell;

class Sheet final : public SheetInterface {
public:
    ~Sheet() noexcept;

    void ClearCell(Position pos) override;
    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;
    Size GetPrintableSize() const noexcept override;
    void PrintTexts(std::ostream& output) const noexcept override;
    void PrintValues(std::ostream& output) const override;
    void SetCell(Position pos, std::string text) override;

private:
    std::unordered_map<Position, std::unique_ptr<Cell>, PosHasher, PosComparator> spreadsheet_;
};