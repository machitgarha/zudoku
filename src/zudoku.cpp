#include "zudoku.hpp"

#include <stdexcept>
#include <string>

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
                this->setValueExistInBlocks({i, j}, this->table[i][j]);
            }
        }
    }

    return *this;
}

SudokuSolver::This SudokuSolver::setValueExistInBlocks(
    const CellIndex &index,
    const CellValue &value,
    bool existNewState
) {
    for (BlockSetData &b: this->blockSetDataArray) {
        bool &existCurState = this->doesValueExistInBlock(b, index, value);

        if (existCurState == existNewState) {
            throw std::invalid_argument(flossy::format(
                "Two equal values encountered in {} {} of the table (value: {})",
                b.name,
                static_cast<unsigned int>(b.indexGetter(index)),
                static_cast<unsigned int>(value)
            ));
        }
        existCurState = existNewState;
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

            // Reverting back the recent steps
            curEmptyCell.possibilities.untried.swap(curEmptyCell.possibilities.tried);

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
    this->setValueExistInBlocks(index, newValue);

    this->table[index.first][index.second] = newValue;

    return *this;
}

SudokuSolver::This SudokuSolver::clearCell(const CellIndex &index)
{
    CellValue &curValue = this->table[index.first][index.second];

    if (!this->isCellEmpty(index)) {
        this->setValueExistInBlocks(index, curValue, false);
        curValue = 0;
    }

    return *this;
}

bool SudokuSolver::isCellEmpty(const CellIndex &index) const noexcept
{
    return this->table[index.first][index.second] == 0;
}

SudokuSolver::NextCorrectPossibility SudokuSolver::findNextCorrectPossibility(
    EmptyCellData &cell
) const {
    NextCorrectPossibility possibility = {false};

    while (!possibility.found && !cell.possibilities.untried.empty()) {
        CellValue value = cell.possibilities.untried.move_top();

        if (!this->doesValueExistInAnySharedBlocks(cell.index, value)) {
            possibility.found = true;
            possibility.value = value;
        }

        cell.possibilities.tried.push(std::move(value));
    }

    return possibility;
}
