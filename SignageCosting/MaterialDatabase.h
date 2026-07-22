#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include "Material.h"

//DEFINITION//
// ----------------- //
//It controls:
//what materials are available(VINYL, CHROMADEK, etc.)
//whether a material exists or not
//what variants each material has
//how you fetch those variants safely

class MaterialDatabase
{
public:
    // ---------------- CORE ----------------
    static void load(const std::string& jsonData);

    static Material get(const std::string& baseId);
    static bool exists(const std::string& baseId);

    // ---------------- BASE SELECTION ----------------
    static std::vector<std::string> getBaseMaterials();

    // ---------------- VARIANT ACCESS ----------------
    static std::vector<MaterialVariant> getVariants(const std::string& baseId);

    static MaterialVariant getVariant(const std::string& baseId, int index);

private:
    static std::unordered_map<std::string, Material> materials;
};