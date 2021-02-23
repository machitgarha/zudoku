#include "app.hpp"

#include "chop.hpp"

using namespace MAChitgarha::Chop;

int main()
{
    try {
        Zudoku::App{}.run();
    } catch (std::exception &e) {
        printLine();
        printLine("Oops, something went wrong:");
        printLine(e.what());
    }

    return 0;
}
