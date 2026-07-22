#pragma once
#include <vector>
#include "Material.h"

class MaterialLoader {
public:
    static std::vector<Material> loadFromJSON(const std::string& jsonData);
};