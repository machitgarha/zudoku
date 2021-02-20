#include "zudoku.hpp"

using namespace Zudoku;

SudokuSolver::SudokuSolver()
{
}

SudokuSolver::SudokuSolver(Table table):
    table(table)
{
}

SudokuSolver::SudokuSolver(Table &&table):
    table(std::move(table))
{
}
