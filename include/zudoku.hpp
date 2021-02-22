#ifndef ZUDOKU_SUDOKU_SOLVER_HPP
#define ZUDOKU_SUDOKU_SOLVER_HPP

#include <array>
#include <string>

#include "stack.hpp"

namespace Zudoku
{
    class SudokuSolver
    {
    private:
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

        /**
         * Constructs the table without validating it. The validation is done in-place when
         * solve() function is called.
         */
        SudokuSolver(Table);
        SudokuSolver(Table &&);

        This solve();

        Table getTable() const;

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
            using Possibilities = stack<CellValue>;

            const CellIndex index;

            struct {
                Possibilities tried, untried;
            } possibilities;
        };

        /**
         * Tells whether a value exist in a block or not. Index 0 should be redundant.
         */
        using ValueExistence = std::array<bool, 10>;

        /**
         * Data of a set of same-type blocks. Block is either a row, a column or a 3x3
         * square, that is a collection of exactly 9 unique cells, filled with 1 to 9.
         *
         * valueExist takes care of existence of values in all blocks one by one. Every
         * block is indexed, and a specific one could be catched using its index.
         *
         * The function (i.e. indexGetter) specified in order to find block index based on
         * a cell index. For example, the cell (0, 0) is in row 0, column 0, and square 0.
         * Note that, squares start from top-left and ends in bottom-right, iterating
         * right-to-left and then up-to-down.
         *
         * The sole definition purpose of the name property is for exception message
         * generation.
         */
        struct BlockSetData
        {
            const std::string name;
            std::array<ValueExistence, 9> valueExist;
            CellLinearIndex (&indexGetter)(const CellIndex &);
        };

        struct {
            stack<EmptyCellData> toBeFilled, filled;
        } emptyCells;

        /**
         * By using an array instead of a struct, iteration will be just easier.
         */
        std::array<BlockSetData, 3> blockSetDataArray = {{
            {"row", {0}, Self::getRowIndex},
            {"column", {0}, Self::getColumnIndex},
            {"square", {0}, Self::getSquareIndex},
        }};

        static void validateCellValue(const CellValue &);
        static void validateCellIndex(const CellIndex &);

        constexpr static CellLinearIndex getRowIndex(const CellIndex &index)
        {
            return index.first;
        }
        constexpr static CellLinearIndex getColumnIndex(const CellIndex &index)
        {
            return index.second;
        }
        constexpr static CellLinearIndex getSquareIndex(const CellIndex &index)
        {
            return index.first / 3 * 3 + index.second / 3;
        }

        constexpr static bool valueExistInBlock(
            const BlockSetData &blockSetData,
            const CellIndex &index,
            const CellValue &value
        ) {
            return blockSetData.valueExist[blockSetData.indexGetter(index)][value];
        }
        constexpr static bool &valueExistInBlock(
            BlockSetData &blockSetData,
            const CellIndex &index,
            const CellValue &value
        ) {
            return blockSetData.valueExist[blockSetData.indexGetter(index)][value];
        }

        constexpr bool valueExistInAllSharedBlocks(
            const CellIndex &index,
            const CellValue &value
        ) const {
            for (const BlockSetData &b : this->blockSetDataArray) {
                if (Self::valueExistInBlock(b, index, value)) {
                    return true;
                }
            }
            return false;
        }

        This makeEmptyCellsAndBlocksData();

        template<bool ValidateIndex = true, bool ValidateValue = true>
        This makeValueVisibleToBlocks(const CellIndex &, const CellValue &);

        This makeEmptyCellsPossibilities();
        This tryEmptyCellsPossibilities();

        This fillCell(const CellIndex &, const CellValue &);
        This clearCell(const CellIndex &);
        bool isCellEmpty(const CellIndex &) const noexcept;

        constexpr This clearCellIfNotEmpty(const CellIndex &index)
        {
            if (!this->isCellEmpty(index)) {
                this->clearCell(index);
            }
            return *this;
        }

        constexpr This replaceCell(const CellIndex &index, const CellValue &value)
        {
            (*this)
                .clearCellIfNotEmpty(index)
                .fillCell(index, value);
            return *this;
        }

    private:
        struct NextCorrectPossibility
        {
            bool found;
            CellValue value = 0;
        };

        Table table;

        NextCorrectPossibility findNextCorrectPossibility(EmptyCellData &) const;
    };
}

#endif // ZUDOKU_SUDOKU_SOLVER_HPP
