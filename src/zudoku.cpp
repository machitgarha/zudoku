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
        .makeEmptyCellsAndBlocksData()
        .makeEmptyCellsPossibilities()
        .tryEmptyCellsPossibilities();

    return *this;
}

SudokuSolver::Table SudokuSolver::getTable() const
{
    return this->table;
}

SudokuSolver::This SudokuSolver::makeEmptyCellsAndBlocksData()
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
        bool &valueExist = this->valueExistInBlock(b, index, value);

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

template SudokuSolver::This SudokuSolver::makeValueVisibleToBlocks<false, false>(
    const CellIndex &index,
    const CellValue &value
);
template SudokuSolver::This SudokuSolver::makeValueVisibleToBlocks<false, true>(
    const CellIndex &index,
    const CellValue &value
);
template SudokuSolver::This SudokuSolver::makeValueVisibleToBlocks<true, false>(
    const CellIndex &index,
    const CellValue &value
);
template SudokuSolver::This SudokuSolver::makeValueVisibleToBlocks<true, true>(
    const CellIndex &index,
    const CellValue &value
);

SudokuSolver::This SudokuSolver::makeEmptyCellsPossibilities()
{
    stack<EmptyCellData> helper;

    while (!this->emptyCells.toBeFilled.empty()) {
        EmptyCellData cell = this->emptyCells.toBeFilled.move_top();

        for (size_t i = 1; i <= 9; i++) {
            if (!this->valueExistInAllSharedBlocks(cell.index, i)) {
                cell.possibilities.untried.push(i);
            }
        }

        helper.push(std::move(cell));
    }

    this->emptyCells.toBeFilled.swap(helper);

    return *this;
}

SudokuSolver::This SudokuSolver::tryEmptyCellsPossibilities()
{
    // While filling the table completely
    while (!this->emptyCells.toBeFilled.empty()) {
        EmptyCellData curEmptyCell = this->emptyCells.toBeFilled.move_top();

        NextCorrectPossibility p = this->findNextCorrectPossibility(curEmptyCell);

        if (p.found) {
            this->replaceCell(curEmptyCell.index, p.value);

            this->emptyCells.filled.push(curEmptyCell);
        } else {
            this->clearCellIfNotEmpty(curEmptyCell.index);

            this->emptyCells.toBeFilled.push(std::move(curEmptyCell));
            this->emptyCells.toBeFilled.push(this->emptyCells.filled.move_top());
        }
    }

    return *this;
}
