#include <iostream>
#include <string_view>
#include <windows.h>
#include "Application.h"

int main(int argc, char* argv[])
{
    const bool developerStartup =
        argc == 2 && std::string_view(argv[1]) == "--dev-startup";
#ifndef _DEBUG
    if (developerStartup)
    {
        std::cerr << "--dev-startup requires a Debug build." << std::endl;
        return 1;
    }
#endif

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

    if (!developerStartup)
    {
        std::cout << "Press ENTER to continue..." << std::endl;
        std::cin.get();
    }

    Application app;

    if (!app.initialise(developerStartup))
        return 1;

    app.run();

    return 0;
}