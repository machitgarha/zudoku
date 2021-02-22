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
    for (CellLinearIndex i: CellLinearIndex::forEach()) {
        for (CellLinearIndex j: CellLinearIndex::forEach()) {
            // Checking whether it is empty or not
            if (this->table[i][j] == 0) {
                this->emptyCells.toBeFilled.push({
                    {i, j},
                    {{}, {}}
                });
            } else {
                // Implicit validation is done for the value
                this->makeValueVisibleToBlocks({i, j}, this->table[i][j]);
            }
        }
    }

    return *this;
}

SudokuSolver::This SudokuSolver::makeValueVisibleToBlocks(
    const CellIndex &index,
    const CellValue &value
) {
    for (BlockSetData &b : this->blockSetDataArray) {
        bool &valueExist = this->doesValueExistInBlock(b, index, value);

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

SudokuSolver::This SudokuSolver::makeEmptyCellsPossibilities()
{
    stack<EmptyCellData> helper;

    while (!this->emptyCells.toBeFilled.empty()) {
        EmptyCellData cell = this->emptyCells.toBeFilled.move_top();

        for (size_t i = 1; i <= 9; i++) {
            if (!this->doesValueExistInAnySharedBlocks(cell.index, i)) {
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
            this->clearCell(curEmptyCell.index);

            this->emptyCells.toBeFilled.push(std::move(curEmptyCell));
            this->emptyCells.toBeFilled.push(this->emptyCells.filled.move_top());
        }
    }

    return *this;
}

SudokuSolver::This SudokuSolver::replaceCell(
    const CellIndex &index,
    const CellValue &newValue
) {
    this->clearCell(index);

    CellValue &curValue = this->table[index.first][index.second];

    for (BlockSetData &b: this->blockSetDataArray) {
        b.valueExist[b.indexGetter(index)][newValue] = true;
    }
    curValue = newValue;

    return *this;
}

SudokuSolver::This SudokuSolver::clearCell(const CellIndex &index)
{
    CellValue &curValue = this->table[index.first][index.second];

    if (!this->isCellEmpty(index)) {
        for (BlockSetData &b: this->blockSetDataArray) {
            b.valueExist[b.indexGetter(index)][curValue] = false;
        }
        curValue = 0;
    }

    return *this;
}

bool SudokuSolver::isCellEmpty(const CellIndex &index) const noexcept
{
    return this->table[index.first][index.second] == 0;
}
