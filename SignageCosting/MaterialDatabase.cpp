#include "MaterialDatabase.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <algorithm>
#include <limits>

using json = nlohmann::json;

std::unordered_map<std::string, Material> MaterialDatabase::materials;

static int g_materialCount = 0;
static int g_variantCount = 0;

// ---------------- LOAD ----------------
bool MaterialDatabase::load(const std::string& jsonData)
{
    materials.clear();
    g_materialCount = 0;
    g_variantCount = 0;

    json j;

    try
    {
        j = json::parse(jsonData);
    }
    catch (const std::exception& e)
    {
        std::cerr << "[MaterialDB][FATAL] JSON PARSE ERROR: " << e.what() << "\n";
        return false;
    }

    if (!j.contains("materials") || !j["materials"].is_array())
    {
        std::cerr << "[MaterialDB][FATAL] Missing 'materials' array\n";
        return false;
    }

    for (const auto& item : j["materials"])
    {
        Material m;
        m.id = item.value("id", "");
        m.name = item.value("name", "");
        m.category = item.value("category", "");
        m.cost_model = item.value("cost_model", "");

        if (m.id.empty())
        {
            std::cerr << "[MaterialDB][WARN] Empty ID skipped\n";
            continue;
        }

        if (!item.contains("variants") || !item["variants"].is_array())
        {
            std::cerr << "[MaterialDB][WARN] No variants for " << m.id << "\n";
        }
        else
        {
            for (const auto& v : item["variants"])
            {
                MaterialVariant mv;

                mv.label = v.value("label", "");
                mv.type = v.value("type", "");
                mv.usage = v.value("usage", "");
                mv.labour_factor = v.value("labour_factor", 1.0);
                mv.duration_years = v.value("duration_years", 0);
                mv.cost_per_m2 = v.value("cost_per_m2", 0.0);

                if (mv.label.empty())
                    mv.label = mv.type + " | " + mv.usage;

                mv.roll_widths.clear();

                if (v.contains("formats") && v["formats"].is_array())
                {
                    for (const auto& f : v["formats"])
                    {
                        // ---------------- VINYL ----------------
                        if (f.contains("roll_width_mm"))
                        {
                            int w = f.value("roll_width_mm", 0);
                            if (w > 0)
                                mv.roll_widths.push_back(w);
                        }

                        // ---------------- SHEETS ----------------
                        else if (f.contains("sheet_width_mm"))
                        {
                            SheetFormat s;
                            s.width = f.value("sheet_width_mm", 0);
                            s.height = f.value("sheet_height_mm", 0);

                            if (s.width > 0 && s.height > 0)
                                mv.sheet_formats.push_back(s);
                        }
                    }
                }

                m.variants.push_back(mv);
                g_variantCount++;
            }
        }

        materials[m.id] = m;
        g_materialCount++;
    }

    return !materials.empty();
}

// ---------------- GET (FIXED - REQUIRED FOR LINKER) ----------------
Material MaterialDatabase::get(const std::string& id)
{
    auto it = materials.find(id);

    if (it == materials.end())
    {
        std::cerr << "[MaterialDB][ERROR] Missing material: " << id << "\n";
        return {};
    }

    return it->second;
}

// ---------------- EXISTS ----------------
bool MaterialDatabase::exists(const std::string& id)
{
    return materials.find(id) != materials.end();
}

// ---------------- BASE LIST (FIXED - REQUIRED FOR LINKER) ----------------
std::vector<std::string> MaterialDatabase::getBaseMaterials()
{
    std::vector<std::string> bases;
    bases.reserve(materials.size());

    for (const auto& [id, mat] : materials)
        bases.push_back(id);

    return bases;
}

// ---------------- VARIANTS ----------------
std::vector<MaterialVariant> MaterialDatabase::getVariants(const std::string& id)
{
    auto it = materials.find(id);

    if (it == materials.end())
        return {};

    return it->second.variants;
}
