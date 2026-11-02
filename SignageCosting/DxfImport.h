#pragma once
#include "NestingGeometry.h"
#include <filesystem>

namespace DxfImport
{
    struct Outline
    {
        NestingGeometry::Polygon polygon; // Local coordinates in millimetres.
        std::string name;
        bool circle=false;
    };
    struct Result
    {
        std::vector<Outline> outlines;
        std::vector<std::string> warnings;
        std::string error;
        bool unitless=false;
        bool valid() const { return error.empty() && !outlines.empty(); }
    };
    Result parse(const std::string& text,double unitlessToMillimetres=1);
    Result load(const std::filesystem::path& path,double unitlessToMillimetres=1);
    // Empty path and empty error means the user cancelled the file chooser.
    std::filesystem::path chooseFile(std::string& error);
}
