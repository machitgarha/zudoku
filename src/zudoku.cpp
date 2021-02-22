#include "zudoku.hpp"

#include <stdexcept>
#include <string>

#include "flossy.h"

using namespace Zudoku;

SudokuSolver::SudokuSolver(Table table):
    table(table)
{
}

SudokuSolver::SudokuSolver(Table &&table):
    table(std::move(table))
{
}

SudokuSolver::This SudokuSolver::solve()
{
    (*this)
        .makeEmptyCellsAndValueExistence()
        .makeEmptyCellsPossibilities()
        .tryEmptyCellsPossibilities();

    return *this;
}

SudokuSolver::Table SudokuSolver::getTable() const
{
    return this->table;
}

SudokuSolver::This SudokuSolver::fill(size_t row, size_t column, CellValue value)
{
    this->validateCellIndex({row, column});
    this->validateCellValue(value);

    this->table[row][column] = value;

    return *this;
}

SudokuSolver::This SudokuSolver::clear(size_t row, size_t column)
{
    this->validateCellIndex({row, column});

    this->table[row][column] = 0;

    return *this;
}

void SudokuSolver::validateCellValue(const CellValue &value)
{
    if (value > 9) {
        throw std::invalid_argument(flossy::format(
            "Expected table cell value to be in the range of 0 to 9, got {}", value
        ));
    }
}

void SudokuSolver::validateCellIndex(const CellIndex &index)
{
    bool invalidFirstIndex;

    if ((invalidFirstIndex = index.first > 9) || index.second > 9) {
        throw std::invalid_argument(flossy::format(
            "Expected table {} index to be in the range of 0 to 8, got {}",
            invalidFirstIndex ? "row" : "column",
            invalidFirstIndex ? index.first : index.second
        ));
    }
}

constexpr SudokuSolver::CellLinearIndex SudokuSolver::getRowIndex(const CellIndex &i)
{
    return i.first;
}

constexpr SudokuSolver::CellLinearIndex SudokuSolver::getColumnIndex(const CellIndex &i)
{
    return i.second;
}

constexpr SudokuSolver::CellLinearIndex SudokuSolver::getSquareIndex(const CellIndex &i)
{
    return i.first / 3 * 3 + i.second / 3;
}

constexpr bool SudokuSolver::getIsValueExist(
    const BlockSetData &blockSetData,
    const CellIndex &index,
    const CellValue &value
) {
    return blockSetData.valueExistence[blockSetData.indexGetter(index)][value];
}

constexpr bool &SudokuSolver::getIsValueExist(
    BlockSetData &blockSetData,
    const CellIndex &index,
    const CellValue &value
) {
    return blockSetData.valueExistence[blockSetData.indexGetter(index)][value];
}

SudokuSolver::This SudokuSolver::makeEmptyCellsAndValueExistence()
{
    for (size_t i = 0; i < 9; i++) {
        for (size_t j = 0; j < 9; j++) {
            // Checking whether it is empty or not
            if (this->table[i][j] == 0) {
                this->emptyCells.toBeFilled.push({
                    {i, j},
                    {{}, {}}
                });
            } else {
                // Implicit validation is done for the value
                this->makeValueVisibleToBlocks<false, true>({i, j}, this->table[i][j]);
            }
        }
    }

    return *this;
}

template<bool ValidateIndex, bool ValidateValue>
SudokuSolver::This SudokuSolver::makeValueVisibleToBlocks(
    const CellIndex &index,
    const CellValue &value
) {
    if constexpr (ValidateIndex) {
        this->validateCellIndex(index);
    }
    if constexpr (ValidateValue) {
        this->validateCellValue(value);
    }

    for (BlockSetData &b : this->blockSetDataArray) {
        bool &valueExist = this->getIsValueExist(b, index, value);

        if (valueExist) {
            throw std::invalid_argument(flossy::format(
                "Two equal values found in {} {} of the table (value: {})",
                b.name, b.indexGetter(index), value
            ));
        }
        valueExist = true;
    }

    return *this;
}

extern template SudokuSolver::This SudokuSolver::makeValueVisibleToBlocks<false, false>(
    const CellIndex &index,
    const CellValue &value
);
extern template SudokuSolver::This SudokuSolver::makeValueVisibleToBlocks<false, true>(
    const CellIndex &index,
    const CellValue &value
);
extern template SudokuSolver::This SudokuSolver::makeValueVisibleToBlocks<true, false>(
    const CellIndex &index,
    const CellValue &value
);
extern template SudokuSolver::This SudokuSolver::makeValueVisibleToBlocks<true, true>(
    const CellIndex &index,
    const CellValue &value
);

SudokuSolver::This SudokuSolver::makeEmptyCellsPossibilities()
{
    stack<EmptyCellData> helper;

    while (!this->emptyCells.toBeFilled.empty()) {
        EmptyCellData &&cell = this->emptyCells.toBeFilled.move_top();

        for (size_t i = 1; i <= 9; i++) {
            if (!this->hasValueInSharedBlocks(cell.index, i)) {
                cell.possibilities.untried.push(i);
            }
        }

        helper.push(cell);
    }

    this->emptyCells.toBeFilled.swap(helper);

    return *this;
}


SudokuSolver::This SudokuSolver::tryEmptyCellsPossibilities()
{
    // While filling the table completely
    while (!this->emptyCells.toBeFilled.empty()) {
        EmptyCellData &&curEmptyCell = this->emptyCells.toBeFilled.move_top();

        while (!curEmptyCell.possibilities.untried.empty()) {
            CellValue value = curEmptyCell.possibilities.untried.top();

            if (!this->hasValueInSharedBlocks(curEmptyCell.index, value)) {
                this->setCellValue(curEmptyCell.index, value);
                // Go to the next empty cell
                break;
            }

            curEmptyCell.possibilities.untried.pop();
            curEmptyCell.possibilities.tried.push(value);
        }

        if (curEmptyCell.possibilities.untried.empty()) {
            curEmptyCell.possibilities.tried.swap(curEmptyCell.possibilities.untried);

            this->emptyCells.toBeFilled.push(curEmptyCell);
            this->emptyCells.toBeFilled.push(this->emptyCells.filled.move_top());
        } else {
            this->emptyCells.filled.push(curEmptyCell);
        }
    }

    return *this;
}

bool SudokuSolver::hasValueInSharedBlocks(
    const CellIndex &index,
    const CellValue &value
) const {
    for (const BlockSetData &b : this->blockSetDataArray) {
        if (this->getIsValueExist(b, index, value)) {
            return true;
        }
    }
    return false;
}
