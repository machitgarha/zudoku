#ifndef ZUDOKU_APP_HPP
#define ZUDOKU_APP_HPP

#include <string>

#include "rapidcsv.h"
#include "sudoku-solver.hpp"

namespace Zudoku
{
    class App
    {
    private:
        using Self = App;
        using This = Self &;

    public:
        App() = default;

        This run();

    protected:
        class ConsoleIO // static
        {
        public:
            ConsoleIO() = delete;

            static void showInitMessage();

            static bool askToSave();
            static bool askToRepeat();
            static bool askToDisplayTable();

            static std::string getInputCsvFilePath();
            static std::string getOutputCsvFilePath();

        protected:
            static bool askYesOrNo(const std::string &, bool defaultAnswer = true);

            static std::string getNonEmptyInput(const std::string &);
        };

        class Validation // static
        {
        public:
            Validation() = delete;

            static void validateCsvData(const rapidcsv::Document &);
        };

        rapidcsv::Document prepareCsvData(const std::string &);

        Zudoku::SudokuSolver::Table prepareTable(const rapidcsv::Document &);
        Zudoku::SudokuSolver::Table solveTable(Zudoku::SudokuSolver::Table &&);

        This saveToFile(const rapidcsv::Document &, const std::string &);
    };
}

#endif // ZUDOKU_APP_HPP