#include "MaterialLoader.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::vector<Material> MaterialLoader::loadFromJSON(const std::string& jsonData) {

    std::vector<Material> materials;

    json j = json::parse(jsonData);

    for (auto& item : j["materials"]) {

        Material m;

        m.name = item.value("name", "");
        m.costPerUnit = item.value("cost", 0.0);

        // FIXED FIELD NAMES
        m.width = item.value("width", 0.0);
        m.height = item.value("height", 0.0);

        m.isSheet = (item.value("unit", "") == "sheet");

        materials.push_back(m);
    }

    return materials;
}