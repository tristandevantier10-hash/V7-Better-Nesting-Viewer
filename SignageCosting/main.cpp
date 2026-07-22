#include <iostream>
#include <windows.h>
#include "Application.h"

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    SetConsoleMode(
        GetStdHandle(STD_OUTPUT_HANDLE),
        ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING
    );

    std::cout <<
        "==================================================\n"
        "          E & G  S I G N S  CC\n"
        "==================================================\n"
        "\n"
        "           ███████╗  ██████╗\n"
        "           ██╔════╝ ██╔════╝\n"
        "           █████╗   ██║  ███╗\n"
        "           ██╔══╝   ██║   ██║\n"
        "           ███████╗ ╚██████╔╝\n"
        "           ╚══════╝  ╚═════╝\n"
        "\n"
        "        SIGNAGE COSTING SYSTEM\n"
        "            VERSION 3.0\n"
        "\n"
        "==================================================\n"
        << std::endl;

    std::cout << "Press ENTER to continue...\n";
    std::cin.get();

    Application app;

    if (!app.initialise())
        return 1;

    app.run();

    return 0;
}