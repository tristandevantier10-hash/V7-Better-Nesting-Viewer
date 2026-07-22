#pragma once

//DEFINITION//
// ----------------- //
//It defines:
//what a material is
//what a variant is
//what data they are allowed to store

#include <string>
#include <vector>

struct SheetFormat
{
    int width = 0;
    int height = 0;
};

struct MaterialVariant
{
    std::string label;

    std::string type;
    std::string usage;

    int duration_years = 0;
    double cost_per_m2 = 0.0;

    std::vector<int> roll_widths;

    std::vector<SheetFormat> sheet_formats; // ADD THIS

    double labour_factor = 1.0;
};

struct Material
{
    std::string id;
    std::string name;
    std::string category;
    std::string cost_model;
    std::vector<MaterialVariant> variants;
};