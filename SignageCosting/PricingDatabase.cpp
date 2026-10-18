#include "PricingDatabase.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>

using json = nlohmann::json;

double PricingDatabase::markupPercent = 0.0;

// ---------------- cached pricing table ----------------
static json g_pricingRoot;

// ---------------- LOAD ----------------
void PricingDatabase::load(const std::string& jsonData)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // 1 second

    try
    {
        g_pricingRoot = json::parse(jsonData);
    }
    catch (const std::exception& e)
    {
        std::cerr << "[PricingDB][FATAL] JSON parse error: " << e.what() << "\n";
        return;
    }

    // ---------------- MARKUP ----------------
    if (g_pricingRoot.contains("defaults") && g_pricingRoot["defaults"].is_object())
    {
        markupPercent = static_cast<double>(
            g_pricingRoot["defaults"].value("markup_percent", 0)
            );
    }

}

// ---------------- MATERIAL COST LOOKUP ----------------
double PricingDatabase::getMaterialCost(
    const std::string& materialId,
    const std::string& variantLabel)
{

    if (!g_pricingRoot.contains("materials"))
    {
        return 0.0;
    }

    for (const auto& mat : g_pricingRoot["materials"])
    {
        if (mat.value("id", "") != materialId)
            continue;

        if (!mat.contains("pricing"))
        {
            return 0.0;
        }

        for (const auto& p : mat["pricing"])
        {
            std::string label =
                p.value("label",
                    p.value("variant",
                        p.value("type", "")));

            if (label == variantLabel)
            {
                double cost =
                    p.value("cost_per_m2",
                        p.value("cost_per_sheet", 0.0));

                return cost;
            }
        }
    }

    return 0.0;
}

// ---------------- GETTERS ----------------
double PricingDatabase::getMarkupPercent()
{
    return markupPercent;
}
