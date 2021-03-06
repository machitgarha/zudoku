#include "app.hpp"

#include "chop.hpp"

using namespace Zudoku;
using namespace MAChitgarha::Chop;

App::This App::run()
{
    App::ConsoleIO::showInitMessage();

    rapidcsv::Document csvData;

    do {
        do {
            try {
                csvData = this->readCsvFileData(
                    App::ConsoleIO::getInputCsvFilePath()
                );
                break;
            } catch (std::ios_base::failure &) {
                printLine("Error: Could not read the file. Please try again.");
                printLine("Check if you entered the path correctly, the file exists and "
                    "is also accessible (i.e. readable).");
            }
        } while (true);

        printLine();
        printLine("Solving Sudoku table... ");
        SudokuSolver::Table solvedTable = this->solveTable(
            this->prepareTable(csvData)
        );
        printLine("Done!");

        if (App::ConsoleIO::askToDisplayTable()) {
            App::ConsoleIO::displayTable(solvedTable);
        }

        do {
            try {
                if (App::ConsoleIO::askToSave()) {
                    this->saveSolvedTableToCsvFile(
                        csvData, App::ConsoleIO::getOutputCsvFilePath(), solvedTable
                    );
                    printLine("File saved successfully.");
                }
                break;
            } catch (std::ios_base::failure &) {
                printLine("Error: Could not save to the specified file.");
                printLine("Perhaps it is a permission error?");
            }
        } while (true);
    } while (App::ConsoleIO::askToRepeat());

    return *this;
}

void App::ConsoleIO::showInitMessage()
{
    printLine("Welcome to Zudoku (GPLv3-licensed), a fast Sudoku solver.");
}

void App::ConsoleIO::displayTable(const SudokuSolver::Table &table)
{
    printLine();
    for (SudokuSolver::CellLinearIndex &i: SudokuSolver::CellLinearIndex::forEach()) {
        for (SudokuSolver::CellLinearIndex &j: SudokuSolver::CellLinearIndex::forEach()) {
            print(table[i][j], " ");
        }
        printLine();
    }
}

bool App::ConsoleIO::askToSave()
{
    return App::ConsoleIO::askYesOrNo(
        "Would you like to save the results?", true
    );
}

bool App::ConsoleIO::askToRepeat()
{
    return App::ConsoleIO::askYesOrNo(
        "Another Sudoku to solve?", false
    );
}

bool App::ConsoleIO::askToDisplayTable()
{
    return App::ConsoleIO::askYesOrNo(
        "Show solved Sudoku table here?", true
    );
}

std::string App::ConsoleIO::getInputCsvFilePath()
{
    return App::ConsoleIO::getNonEmptyInput(
        "Please enter the path of the input CSV file (including Sudoku table data):"
    );
}

std::string App::ConsoleIO::getOutputCsvFilePath()
{
    return App::ConsoleIO::getNonEmptyInput(
        "Enter the path of the output CSV file:"
    );
}

bool App::ConsoleIO::askYesOrNo(const std::string &question, bool defaultAnswer)
{
    printLine();

    // Ask until a good input is given
    do {
        print(question, " [", defaultAnswer == true ? "Y/n" : "y/N", "] ");

        std::string answer;
        std::getline(std::cin, answer);


        if (answer.empty()) {
            return defaultAnswer;
        }
        if (std::tolower(answer[0]) == 'y') {
            return true;
        }
        if (std::tolower(answer[0]) == 'n') {
            return false;
        }
    } while (true);
}

std::string App::ConsoleIO::getNonEmptyInput(const std::string &message)
{
    std::string input;

    printLine();
    do {
        print(message, " ");
        std::getline(std::cin, input);
    } while (input.empty());

    return input;
}

void App::Validation::validateCsvData(const rapidcsv::Document &csvData)
{
    if (csvData.GetRowCount() != 9 || csvData.GetColumnCount() != 9) {
        throw std::out_of_range(flossy::format(
            "Expected CSV data to be exactly 9x9, but is {}x{}",
            csvData.GetRowCount(),
            csvData.GetColumnCount()
        ));
    }
}

rapidcsv::Document App::readCsvFileData(const std::string &inputCsvFilePath)
{
    rapidcsv::Document csvData{
        inputCsvFilePath, rapidcsv::LabelParams{-1, -1}
    };

    App::Validation::validateCsvData(csvData);

    return csvData;
}

SudokuSolver::Table App::prepareTable(const rapidcsv::Document &csvData)
{
    SudokuSolver::Table table;

    for (size_t i = 0; i < csvData.GetRowCount(); i++) {
        for (size_t j = 0; j < csvData.GetColumnCount(); j++) {
            table[j][i] = csvData.GetCell<unsigned int>(i, j);
        }
    }

    return table;
}

SudokuSolver::Table App::solveTable(SudokuSolver::Table &&table)
{
    return SudokuSolver{table}.solve().getTable();
}

App::This App::saveSolvedTableToCsvFile(
    rapidcsv::Document &csvData,
    const std::string &outputCsvFilePath,
    const SudokuSolver::Table &solvedTable
) {
    for (SudokuSolver::CellLinearIndex &i: SudokuSolver::CellLinearIndex::forEach()) {
        for (SudokuSolver::CellLinearIndex &j: SudokuSolver::CellLinearIndex::forEach()) {
            csvData.SetCell<unsigned int>(i, j, solvedTable[j][i]);
        }
    }

    csvData.Save(outputCsvFilePath);
    return *this;
}
