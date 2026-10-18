#pragma once

#include <string>

class FileSystem
{
public:

    static std::string getDataFolder();

    static std::string getQuotesFolder();

    static void initialise();
};
