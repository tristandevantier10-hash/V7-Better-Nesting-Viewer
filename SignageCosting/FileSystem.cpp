#include "FileSystem.h"

#include <filesystem>

namespace fs = std::filesystem;

void FileSystem::initialise()
{
    fs::create_directories("Data");
    fs::create_directories("Quotes");
}

std::string FileSystem::getDataFolder()
{
    return "Data/";
}

std::string FileSystem::getQuotesFolder()
{
    return "Quotes/";
}
