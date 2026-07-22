#include "MaterialDatabase.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <limits>

using json = nlohmann::json;

std::unordered_map<std::string, Material> MaterialDatabase::materials;

static int g_materialCount = 0;
static int g_variantCount = 0;

// ---------------- LOAD ----------------
void MaterialDatabase::load(const std::string& jsonData)
{
    materials.clear();
    g_materialCount = 0;
    g_variantCount = 0;

    json j;

    //std::cout << "\n[MaterialDB] ===============================\n";
    //std::cout << "[MaterialDB] START LOAD\n";
    //std::cout << "[MaterialDB] RAW SIZE: " << jsonData.size() << " bytes\n";
    std::cout << "[OK] Materials database loaded\n\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // 1 second

    try
    {
        j = json::parse(jsonData);
    }
    catch (const std::exception& e)
    {
        std::cerr << "[MaterialDB][FATAL] JSON PARSE ERROR: " << e.what() << "\n";
        return;
    }

    if (!j.contains("materials") || !j["materials"].is_array())
    {
        std::cerr << "[MaterialDB][FATAL] Missing 'materials' array\n";
        return;
    }

    //std::cout << "[MaterialDB] Materials found: " << j["materials"].size() << "\n";

    int materialIndex = 0;

    for (const auto& item : j["materials"])
    {
        //std::cout << "\n[MaterialDB] --------------------------------\n";
        //std::cout << "[MaterialDB] Material index: " << materialIndex++ << "\n";

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

        //std::cout << "[MaterialDB] ID: " << m.id << "\n";
        //std::cout << "   Name: " << m.name << "\n";
        //std::cout << "   Category: " << m.category << "\n";

        if (!item.contains("variants") || !item["variants"].is_array())
        {
            std::cerr << "[MaterialDB][WARN] No variants for " << m.id << "\n";
        }
        else
        {
            int variantIndex = 0;

            for (const auto& v : item["variants"])
            {
                //std::cout << "   [Variant " << variantIndex++ << "]\n";

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

    //std::cout << "\n[MaterialDB] LOAD COMPLETE\n";
    //std::cout << "[MaterialDB] Materials: " << g_materialCount << "\n";
    //std::cout << "[MaterialDB] Variants: " << g_variantCount << "\n";
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