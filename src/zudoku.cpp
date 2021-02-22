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

    if ((invalidFirstIndex = index.first > 9) || index.second > 9) {
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

    struct BlockSetData
    {
        /**
         * Used only for exception message generation.
         */
        const std::string name;

        /**
         * Used to find the index of the correspending block. For example, if we need to
         * make cell (0, 0) visible to its row, we need to know its index (which is 0).
         */
        CellLinearIndex (&indexGetter)(const CellIndex &);
    };

    std::array<BlockSetData, 3> blockSetDataArray = {{
        {"row", this->valueExistence.rows, Self::getRowIndex},
        {"column", this->valueExistence.columns, Self::getColumnIndex},
        {"square", this->valueExistence.squares, Self::getSquareIndex},
    }};

    for (const BlockSetData &b : blockSetDataArray) {
        bool &valueExist = b.valueExistence[b.indexGetter(index)][value];

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
