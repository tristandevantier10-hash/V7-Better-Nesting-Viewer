#include "FileSystem.h"

#include <filesystem>

namespace fs = std::filesystem;

void FileSystem::initialise()
{
    fs::create_directories("Data");
}

std::string FileSystem::getDataFolder()
{
    return "Data/";
}