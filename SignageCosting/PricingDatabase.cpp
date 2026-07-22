#include "PricingDatabase.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>

using json = nlohmann::json;

// ================= DEBUG SWITCH =================
static constexpr bool DEBUG_PRICING = false;

#if DEBUG_PRICING
#define PRICE_DEBUG(msg) std::cout << msg << std::endl;
#else
#define PRICE_DEBUG(msg)
#endif

double PricingDatabase::labourPerM2 = 0.0;
double PricingDatabase::markupPercent = 0.0;

// ---------------- cached pricing table ----------------
static json g_pricingRoot;

// ---------------- LOAD ----------------
void PricingDatabase::load(const std::string& jsonData)
{
    //std::cout << "\n[PricingDB] ===============================\n";
    //std::cout << "[PricingDB] START LOAD\n";
    //std::cout << "[PricingDB] RAW SIZE: " << jsonData.size() << " bytes\n";
    std::cout << "[OK] Pricing database loaded\n\n";
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

    PRICE_DEBUG("Parsing pricing JSON...");

    // ---------------- LABOUR ----------------
    if (g_pricingRoot.contains("labour") && g_pricingRoot["labour"].is_object())
    {
        labourPerM2 = static_cast<double>(
            g_pricingRoot["labour"].value("cost_per_m2", 0)
            );

        PRICE_DEBUG("Labour per m2 = " + std::to_string(labourPerM2));
    }

    // ---------------- MARKUP ----------------
    if (g_pricingRoot.contains("defaults") && g_pricingRoot["defaults"].is_object())
    {
        markupPercent = static_cast<double>(
            g_pricingRoot["defaults"].value("markup_percent", 0)
            );
        PRICE_DEBUG("Markup % = " + std::to_string(markupPercent));
    }

    PRICE_DEBUG("Pricing load complete");

    std::cout << "   Labour: " << labourPerM2 << "\n";
    std::cout << "   Markup: " << markupPercent << "\n\n";
}

// ---------------- MATERIAL COST LOOKUP ----------------
double PricingDatabase::getMaterialCost(
    const std::string& materialId,
    const std::string& variantLabel)
{
    PRICE_DEBUG("================================================");
    PRICE_DEBUG("REQUEST => " + materialId + " | " + variantLabel);

    if (!g_pricingRoot.contains("materials"))
    {
        PRICE_DEBUG("ERROR: No 'materials' block found");
        return 0.0;
    }

    for (const auto& mat : g_pricingRoot["materials"])
    {
        if (mat.value("id", "") != materialId)
            continue;

        if (!mat.contains("pricing"))
        {
            PRICE_DEBUG("ERROR: Material missing pricing array");
            return 0.0;
        }

        PRICE_DEBUG("---- ENTERING PRICING ARRAY ----");

        for (const auto& p : mat["pricing"])
        {
            std::string label =
                p.value("label",
                    p.value("variant",
                        p.value("type", "")));

            PRICE_DEBUG("EXTRACTED LABEL => '" + label + "'");
            PRICE_DEBUG("COMPARE => '" + label + "' vs '" + variantLabel + "'");

            if (label == variantLabel)
            {
                double cost =
                    p.value("cost_per_m2",
                        p.value("cost_per_sheet", 0.0));

                PRICE_DEBUG("MATCH FOUND => " + std::to_string(cost));
                return cost;
            }
        }
    }

    PRICE_DEBUG("NO MATCH FOUND => " + materialId + " | " + variantLabel);
    return 0.0;
}

// ---------------- GETTERS ----------------
double PricingDatabase::getLabourPerM2()
{
    return labourPerM2;
}

double PricingDatabase::getMarkupPercent()
{
    return markupPercent;
}