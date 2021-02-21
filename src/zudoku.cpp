#include "zudoku.hpp"

#include <stdexcept>
#include <string>

#include "flossy.h"

using namespace Zudoku;

SudokuSolver::SudokuSolver(Table table):
    table(table)
{
    this->validateTable(table);
}

SudokuSolver::SudokuSolver(Table &&table):
    table(std::move(table))
{
    this->validateTable(table);
}

SudokuSolver::This SudokuSolver::setTable(Table table)
{
    this->validateTable(table);

    this->table = table;

    return *this;
}

SudokuSolver::This SudokuSolver::setTable(Table &&table)
{
    this->validateTable(table);

    this->table = std::move(table);

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

void SudokuSolver::validateTable(const Table &table)
{
    for (size_t i = 0; i < table.size(); i++) {
        for (size_t j = 0; j < table[i].size(); j++) {
            try {
                Self::validateCellValue(table[i][j]);
            } catch (std::invalid_argument &e) {
                throw std::invalid_argument(flossy::format(
                    std::string(e.what()) + ", at index ({}, {})", i, j
                ));
            }
        }
    }
}
