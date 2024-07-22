#pragma once

#include <iosfwd>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

struct Position final {
    int row = 0;
    int col = 0;

    bool operator==(Position rhs) const;
    bool operator<(Position rhs) const;

    bool IsValid() const noexcept;
    std::string ToString() const;

    static Position FromString(std::string_view str);

    static const int MAX_ROWS = 16384;
    static const int MAX_COLS = 16384;
    static const Position NONE;
};

struct Size final {
    int rows = 0;
    int cols = 0;

    bool operator==(Size rhs) const noexcept;
};

class FormulaError final {
public:
    enum class Category {
        Ref,
        Value,
        Arithmetic,
    };

    FormulaError(Category category);
    bool operator==(FormulaError rhs) const noexcept;

    Category GetCategory() const;
    std::string_view ToString() const;

private:
    Category category_;
};

std::ostream& operator<<(std::ostream& output, FormulaError fe);

class InvalidPositionException final : public std::out_of_range {
public:
    using std::out_of_range::out_of_range;
};

class FormulaException final : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class CircularDependencyException final : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class CellInterface {
public:
    using Value = std::variant<std::string, double, FormulaError>;

    virtual ~CellInterface() noexcept = default;

    virtual std::string GetText() const noexcept = 0;
    virtual Value GetValue() const = 0;
    virtual std::vector<Position> GetReferencedCells() const = 0;
};

inline constexpr char FORMULA_SIGN = '=';
inline constexpr char ESCAPE_SIGN = '\'';

class SheetInterface {
public:
    virtual ~SheetInterface() = default;

    virtual void ClearCell(Position pos) = 0;
    virtual const CellInterface* GetCell(Position pos) const = 0;
    virtual CellInterface* GetCell(Position pos) = 0;
    virtual Size GetPrintableSize() const noexcept = 0;
    virtual void PrintTexts(std::ostream& output) const noexcept = 0;
    virtual void PrintValues(std::ostream& output) const = 0;
    virtual void SetCell(Position pos, std::string text) = 0;
};

std::unique_ptr<SheetInterface> CreateSheet();