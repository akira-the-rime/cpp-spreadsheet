#include <deque>
#include <utility>

#include "cell.h"

Cell::Cell(Sheet& spreadsheet)
    : impl_(nullptr)
    , spreadsheet_(spreadsheet) {
}

Cell::~Cell() noexcept = default;

void Cell::Clear() noexcept {
    impl_ = std::make_unique<detail::EmptyImpl>("");
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

std::string Cell::GetText() const noexcept {
    return impl_->GetText();
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}

bool Cell::HasUpperLevel() const {
    return !upper_level_.empty();
}

void Cell::Set(std::string text) {
    bool update_statement = true;

    if ((text.size() == 1 && (text.front() == ESCAPE_SIGN || text.front() == FORMULA_SIGN)) || text.empty()) {
        impl_ = std::make_unique< detail::EmptyImpl>(std::move(text));
    }
    else if (text.front() == FORMULA_SIGN) {
        std::unique_ptr<detail::Impl> being_considered_impl = std::make_unique<detail::FormulaImpl>(std::move(text), spreadsheet_);

        if (this->impl_ != nullptr && (impl_->GetText() == being_considered_impl->GetText())) {
            update_statement = false;
        }

        if (update_statement) {
            if (!being_considered_impl->GetReferencedCells().empty() 
                && CheckOnCyclicDependency(being_considered_impl.get())) {

                throw CircularDependencyException("Cyclic dependency was met.");
            }

            impl_ = std::move(being_considered_impl);
        }
    }
    else {
        impl_ = std::make_unique<detail::TextImpl>(std::move(text));
    }

    if (update_statement) {
        AdjustCellsDependency(impl_.get());
        InvalidateCache(upper_level_);
    }
}

void Cell::AdjustCellsDependency(const detail::Impl* const being_considered_impl) {
    for (Cell* lower_cell : lower_level_) {
        lower_cell->upper_level_.erase(this);
    }
    lower_level_.clear();

    for (const Position& cell_position : being_considered_impl->GetReferencedCells()) {
        CellInterface* taken_cell = spreadsheet_.GetCell(cell_position);

        if (taken_cell == nullptr) {
            spreadsheet_.SetCell(cell_position, "");
            taken_cell = spreadsheet_.GetCell(cell_position);
        }

        Cell* casted_cell = static_cast<Cell*>(taken_cell);
        casted_cell->upper_level_.emplace(this);
        lower_level_.emplace(casted_cell);
    }
}

bool Cell::CheckOnCyclicDependency(const detail::Impl* const being_considered_impl) {
    const std::unordered_set<const CellInterface*> impl_cells = [&] {
        std::unordered_set<const CellInterface*> temporary;

        for (const Position& cell_position : being_considered_impl->GetReferencedCells()) {
            if (const CellInterface* const taken_cell = spreadsheet_.GetCell(cell_position); taken_cell != nullptr) {
                temporary.emplace(taken_cell);
            }
        }

        return temporary;
    }();

    std::unordered_set<const CellInterface*> encountered;
    std::deque<const CellInterface*> to_check = { this };

    while (!to_check.empty()) {
        const CellInterface* const taken_cell = to_check.front();

        if (impl_cells.count(taken_cell)) {
            return true;
        }
        to_check.pop_front();
        encountered.emplace(taken_cell);

        for (const CellInterface* const upper_cell : static_cast<const Cell*>(taken_cell)->upper_level_) {
            if (!encountered.count(upper_cell)) {
                to_check.emplace_back(upper_cell);
            }
        }
    }

    return false;
}

void Cell::InvalidateCache(const std::unordered_set<Cell*>& to_invalidate) {
    for (Cell* cell : to_invalidate) {
        static_cast<detail::FormulaImpl*>(cell->impl_.get())->InvalidateCache();

        InvalidateCache(cell->upper_level_);
    }
}