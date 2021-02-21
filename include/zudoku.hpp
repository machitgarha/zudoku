#include <array>
#include <stack>

namespace Zudoku
{
    class SudokuSolver
    {
    public:
        using This = SudokuSolver &;
        using Self = SudokuSolver;

        using CellLinearIndex = unsigned int;
        using CellIndex = std::pair<CellLinearIndex, CellLinearIndex>;

        using CellValue = unsigned int;

        /**
         * A sudoku table. For the value of each cell, number in the range of 1 to 9
         * obviously means the cell is filled, 0 means the cell is empty, and all other
         * values are invalid and causes an exception to be thrown.
         */
        using Table = std::array<std::array<CellValue, 9>, 9>;

        SudokuSolver() = delete;

        SudokuSolver(Table);
        SudokuSolver(Table &&);

        This solve();

        This setTable(Table);
        This setTable(Table &&);
        Table getTable() const;

        This fill(size_t, size_t, CellValue);
        This clear(size_t, size_t);

    protected:
        /*
         * The algorithm to fill the table is as follows:
         *
         * Initially, we iterate all table cells, to detect filled and empty ones. A stack
         * of information of empty cells is created, consisting of its index and its
         * possible values, including tried and untried ones. Then, for each empty cell,
         * we (check and) fill it with its first untried possible number, and if we reach a
         * locking state (i.e. where there is no possilility for the current cell), things
         * are reverted back (with the help of tried possililities stack), and the other
         * possibilities are tried, until filling the table correctly and completely.
         *
         * Note that, untried possibilities is only to eliminate the obvious choices (i.e.
         * the ones statically filled). A dynamic check is also needed when filling the
         * table, whether the current possibility is used by a preivous cell or not.
         *
         * For instance, consider the first cell possililities as {2, 7, 8} and the second
         * one as {2, 3, 5, 8}, and suppose them in one row. If we choose 2 for the first
         * cell, then it is obvious that the second cell must not be 2, and many other
         * cells as well.
         *
         * To overcome this problem, there are some methods. One is to iterate over all
         * affected cells and their possilility stacks, and modify them. The cost of this
         * operation as a whole is too high, whether the possibilities stack is sorted or
         * not. We must (1) find the number in the stack and pop it, which is costly if it
         * lives at the bottom of the stack, and (2) revert back all modified stacks in the
         * case of wrong path, which too many unnecessary changes might be made in this
         * case.
         *
         * A better solution is to have one array for each row, column and 3x3 square,
         * holding the availability of numbers in it. Now, after filling a cell, we change
         * only three values (constant order), reserving the filled number in the related
         * row, column and square. Then, for all (future) cells, we check three values for
         * each untried possililities before filling them. This way, reverting back is as
         * simple as reverting the changed values, and there is almost zero unnecessary
         * changes to be reverted.
         */

        /**
         * The type representing data of an empty cell in the Sudoku table, consisting of
         * its index and its tried and untried possibilities. Keeping tried possibilities
         * is necessary to revert back from a wrong try path.
         */
        struct EmptyCellData
        {
            using Possibilities = std::stack<CellValue>;

            CellIndex index;

            struct {
                Possibilities tried, untried;
            } possibilities;
        };

        /**
         * Tells whether a value exist in a block or not. By block, we mean either a row,
         * a column, or a 3x3 square. Index 0 should be redundant.
         */
        using ValueExistence = std::array<bool, 10>;

        struct {
            std::stack<EmptyCellData> toBeFilled, filled;
        } emptyCells;

        struct {
            std::array<ValueExistence, 9> rows, columns, squares;
        } valueExistence;

        static void validateCellValue(const CellValue &);
        static void validateCellIndex(const CellIndex &);
        static void validateTable(const Table &);

        This setEmptyCellsAndValueExistence();
        This makeValueExist(const CellIndex &, const CellValue &);

        This setEmptyCellsPossibilities();
        This tryEmptyCellsPossibilities();

    private:
        Table table;
    };
}
