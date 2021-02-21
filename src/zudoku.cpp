#include "zudoku.hpp"

#include <stdexcept>
#include <string>

#include "flossy.h"

using namespace Zudoku;
using namespace std::string_literals;

SudokuSolver::SudokuSolver()
{
}

SudokuSolver::SudokuSolver(Table table):
    table(table)
{
    this->validateTable(table);
}

SudokuSolver::SudokuSolver(Table &&table):
    table(std::move(table))
{
}

SudokuSolver::This SudokuSolver::setTable(Table table)
{
    this->table = table;
    this->validateTable(table);

    return *this;
}

SudokuSolver::This SudokuSolver::setTable(Table &&table)
{
    this->table = std::move(table);
    this->validateTable(table);

    return *this;
}

SudokuSolver::Table SudokuSolver::getTable() const
{
    return this->table;
}

void SudokuSolver::validateTable(const Table &table)
{
    for (size_t i = 0; i < table.size(); i++) {
        for (size_t j = 0; j < table[i].size(); j++) {
            if (table[i][j] > 9) {
                throw std::invalid_argument(flossy::format(
                    "Expected table cell value to be in range 0 to 9, got {}, at index "
                    "({}, {})", table[i][j], i, j
                ));
            }
        }
    }
}
