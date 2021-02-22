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
    this->makeEmptyCellsAndValueExistence();
    this->makeEmptyCellsPossibilities();
    this->tryEmptyCellsPossibilities();

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

    if ((invalidFirstIndex = index.first > 8) || index.second > 8) {
        throw std::invalid_argument(flossy::format(
            "Expected table {} index to be in the range of 0 to 8, got {}",
            invalidFirstIndex ? "row" : "column",
            invalidFirstIndex ? index.first : index.second
        ));
    }
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

    this->valueExistence.rows[this->getRowIndex(index)] = value;
    this->valueExistence.columns[this->getColumnIndex(index)] = value;
    this->valueExistence.squares[this->getSquareIndex(index)] = value;

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
