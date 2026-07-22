#include "CostEngine.h"
#include "PricingRules.h"
#include "NestingEngine.h"
#include "PricingDatabase.h"
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <cctype>
#include "NestingCalculator.h"
#include "RollOptimizer.h"
#include "ProductionPricingDatabase.h"
#include "NestingCore.h"
#include <cmath>
#include "NestGroup.h"
#include "NestingManager.h"

// ---------------- MAIN ENGINE ----------------

CostResult CostEngine::calculate(Job job)
{
    CostResult result;
    NestingEngine nesting;
    NestingManager nestingManager;

    auto nestingResult =
        nestingManager.calculate(job);

    result.nestingSheets = nestingResult.sheets;

    result.materialCost = 0.0;
    result.labourCost = 0.0;
    result.productionCost = 0.0;
    result.sellPrice = 0.0;

    bool useCoreNesting = true;   // toggle stays for safety

    const double labourRate = PricingDatabase::getLabourPerM2();
    const double markup = PricingDatabase::getMarkupPercent();

    int jobCounter = 1;

    for (size_t i = 0; i < job.items.size(); i++)
    {
        auto& item = job.items[i];
        const Material& m = item.material;

        std::cout << "\n===== COST ENGINE =====\n";
        std::cout << "Category : " << item.material.category << "\n";
        std::cout << "Roll Width : " << item.selectedRollWidth << "\n";
        std::cout << "Sheet Width : " << item.selectedSheetFormat.width << "\n";
        std::cout << "Sheet Height: " << item.selectedSheetFormat.height << "\n";

        if (m.variants.empty())
        {
            std::cerr << "[CRITICAL] No variants for " << m.id << "\n";
            continue;
        }

        if (item.variantIndex < 0 || item.variantIndex >= (int)m.variants.size())
        {
            std::cerr << "[CRITICAL] Invalid variant index\n";
            continue;
        }

        const MaterialVariant& v = m.variants[item.variantIndex];

        NestGroup* currentGroup = nullptr;

        for (auto& group : nestingResult.groups)
        {
            if (std::find(
                group.jobItemIndices.begin(),
                group.jobItemIndices.end(),
                static_cast<int>(i))
                != group.jobItemIndices.end())
            {
                currentGroup = &group;
                break;
            }
        }

        ItemCostResult itemResult{};

        itemResult.width = item.width;
        itemResult.height = item.height;

        CostTrace trace;
        trace.materialId = m.id;
        trace.variantLabel = v.label;
        trace.width = item.width;
        trace.height = item.height;
        trace.qty = item.quantity;

        const double areaPerItem = PricingRules::calculateArea(item.width, item.height);
        const double totalArea = areaPerItem * item.quantity;

        const double baseJobCost = 50.0;

        const double materialFactor =
            (v.labour_factor > 0.0) ? v.labour_factor : 1.0;

        const double labourCost =
            baseJobCost + (totalArea * labourRate * materialFactor);

        double materialCost = 0.0;
        double productionCostCalc = 0.0;

        // ---------------- PRODUCTION ----------------
        if (item.production.print)
            productionCostCalc += totalArea * ProductionPricingDatabase::getPrintRate();

        if (item.production.laminate)
            productionCostCalc += totalArea * ProductionPricingDatabase::getLaminateRate();

        if (item.production.plotterCut)
            productionCostCalc += item.quantity * ProductionPricingDatabase::getPlotterCutRate();

        if (item.production.routerCut)
            productionCostCalc += item.quantity * ProductionPricingDatabase::getRouterCutRate();

        if (item.production.application)
            productionCostCalc += totalArea * ProductionPricingDatabase::getApplicationRate();

        if (item.production.frame)
            productionCostCalc += totalArea * ProductionPricingDatabase::getFrameRate();

        // =====================================================
        // ROLL AREA (UNCHANGED)
        // =====================================================

        std::string rule = m.cost_model;
        double resolvedRoll = item.selectedRollWidth;

        RollSolution best{};

        if (rule == "ROLL_AREA")
        {
            calculateRollItem(
                item,
                itemResult,
                result,
                totalArea,
                materialCost);
        }

        // =====================================================
        // SHEET AREA (CORE INTEGRATION ONLY HERE)
        // =====================================================

        else if (rule == "SHEET_AREA")
        {
            calculateSheetItem(
                item,
                *currentGroup,
                itemResult,
                result,
                totalArea,
                materialCost);
        }

        // fallback
        else
        {
            calculateGenericItem(
                item,
                totalArea,
                materialCost);
        }

        trace.materialCost = materialCost;

        // =====================================================
        // FINAL COSTING (UNCHANGED)
        // =====================================================
        double baseCost = materialCost + labourCost + productionCostCalc;
        double markupValue = baseCost * (markup / 100.0);
        double sellPrice = baseCost + markupValue;

        itemResult.sellPrice = sellPrice;   // ADD THIS
        itemResult.markupValue = markupValue;   // ADD THIS
        result.materialCost += materialCost;
        result.labourCost += labourCost;
        result.sellPrice += sellPrice;
        result.productionCost += productionCostCalc;

        itemResult.materialId = m.id;
        itemResult.category = m.category;
        itemResult.variant = v.label;
        itemResult.quantity = item.quantity;
        itemResult.area = totalArea;

        itemResult.materialCost = materialCost;
        itemResult.labourCost = labourCost;
        itemResult.production = item.production;
        itemResult.productionCost = productionCostCalc;

        result.items.push_back(itemResult);
    }

    result.totalCost =
        result.materialCost +
        result.labourCost +
        result.productionCost;

    // sellPrice already accumulated per item, so margin is valid
    result.margin = result.sellPrice - result.totalCost;

    std::cout << "\n========== NEST GROUPS ==========\n";

    for (const auto& group : nestingResult.groups)
    {
        std::cout << "\nMaterial : " << group.materialId << "\n";
        std::cout << "Variant  : " << group.variant << "\n";

        if (group.isRoll)
            std::cout << "Roll     : " << group.rollWidth << "\n";
        else
            std::cout << "Sheet    : "
            << group.sheetWidth
            << " x "
            << group.sheetHeight
            << "\n";

        std::cout << "Rects:\n";

        for (const auto& r : group.rects)
        {
            std::cout
                << "   "
                << r.width
                << " x "
                << r.height
                << " Qty "
                << r.quantity
                << "\n";
        }
    }

    return result;
}

void CostEngine::calculateGenericItem(
    const JobItem& item,
    double totalArea,
    double& materialCost)
{
    const Material& m = item.material;

    const MaterialVariant& v =
        m.variants[item.variantIndex];

    const double unitCost =
        PricingDatabase::getMaterialCost(
            m.id,
            v.label);

    materialCost = totalArea * unitCost;
}

void CostEngine::calculateRollItem(
    const JobItem& item,
    ItemCostResult& itemResult,
    CostResult& result,
    double totalArea,
    double& materialCost)
{
    const Material& m = item.material;

    const MaterialVariant& v =
        m.variants[item.variantIndex];

    NestingCore core;

    RollSolution best{};

        std::vector<double> rollOptions;

        if (item.autoRoll)
        {
            // full search space
            rollOptions.assign(v.roll_widths.begin(), v.roll_widths.end());
        }
        else
        {
            // constrained search space (ONLY selected width)
            rollOptions.push_back(item.selectedRollWidth);
        }

        best = RollOptimizer::evaluate(
            item.width,
            item.height,
            item.quantity,
            rollOptions
        );

        double localResolvedRoll = best.rollWidth;
        double rollWidthM = localResolvedRoll / 1000.0;

        double requiredMeters = best.lengthUsed;

        best.wasteArea =
            (best.rollWidth / 1000.0 * requiredMeters) - totalArea;

        best.efficiency =
            (totalArea /
                (best.rollWidth / 1000.0 * requiredMeters)) * 100.0;

        itemResult.rollSolution = best;
        itemResult.rollWidth = localResolvedRoll;

        Rect r
        {
            (int)item.width,
            (int)item.height,
            item.quantity
        };

        std::vector<Rect> rects = { r };

        int rollLength =
            static_cast<int>(
                std::ceil(best.lengthUsed * 1000.0));

        std::cout << "\n===== ROLL LAYOUT =====\n";
        std::cout << "Roll Width  : " << best.rollWidth << "\n";
        std::cout << "Length Used : " << best.lengthUsed << " m\n";
        std::cout << "Roll Length : " << rollLength << " mm\n";
        std::cout << "Qty         : " << item.quantity << "\n";
        std::cout << "Item        : "
            << item.width << " x "
            << item.height << "\n";

        std::vector<Sheet> layouts =
            core.pack(
                rects,
                (int)best.rollWidth,
                rollLength,
                PlacementStrategy::BestShortSideFit);

        std::cout << "Sheets Produced: "
            << layouts.size()
            << "\n";

        std::cout << "\n===== ROLL PREVIEW DEBUG =====\n";
        std::cout << "Layouts: " << layouts.size() << "\n";

        for (size_t i = 0; i < layouts.size(); i++)
        {
            std::cout << "Sheet " << i + 1
                << " placed: "
                << layouts[i].placed.size()
                << "\n";
        }

        std::cout
            << "Roll Width : " << best.rollWidth << "\n"
            << "Roll Length: " << rollLength << "\n";

        materialCost = requiredMeters *
            PricingDatabase::getMaterialCost(m.id, v.label);
        }

        void CostEngine::calculateSheetItem(
            const JobItem& item,
            const NestGroup& group,
            ItemCostResult& itemResult,
            CostResult& result,
            double totalArea,
            double& materialCost)
        {

    const Material& m = item.material;

    const MaterialVariant& v =
        m.variants[item.variantIndex];

    const auto& sheetLayouts = group.layouts;

    const int sheetCount =
        static_cast<int>(sheetLayouts.size());

    itemResult.sheetsUsed = sheetCount;

    itemResult.width = item.selectedSheetFormat.width;
    itemResult.height = item.selectedSheetFormat.height;

    const double unitCost =
        PricingDatabase::getMaterialCost(m.id, v.label);

    materialCost = sheetCount * unitCost;

    std::cout << "\n===== MATERIAL DEBUG =====\n";
    std::cout << "Material : " << m.id << "\n";
    std::cout << "Variant  : " << v.label << "\n";
    std::cout << "UnitCost : " << unitCost << "\n";
    std::cout << "Sheets   : " << sheetCount << "\n";
    std::cout << "MaterialCost = " << materialCost << "\n";

    // =====================================================
    // ORIGINAL DEBUG OUTPUT (UNCHANGED STYLE)
    // =====================================================
    std::cout << "\n====================================\n";
    std::cout << "NESTING DEBUG | Material: " << m.id
        << " | Variant: " << v.label << "\n";

    std::cout
        << "Sheet Size        : "
        << item.selectedSheetFormat.width
        << " x "
        << item.selectedSheetFormat.height
        << " mm\n";

    std::cout
        << "Input Object Size : "
        << item.width << " x "
        << item.height
        << " mm\n";

    std::cout
        << "Quantity          : "
        << item.quantity
        << "\n";

    std::cout
        << "Sheets Used       : "
        << sheetCount
        << "\n";

    double sheetArea =
        (
            item.selectedSheetFormat.width *
            item.selectedSheetFormat.height
            ) / 1000000.0;

    double objectArea =
        (item.width * item.height * item.quantity) / 1000000.0;

    std::cout
        << "Object Area       : "
        << objectArea
        << " m2\n";

    std::cout
        << "Single Sheet Area : "
        << sheetArea
        << " m2\n";

    double usedArea =
        objectArea / (sheetArea * sheetCount) * 100.0;

    std::cout
        << "Material Usage    : "
        << usedArea
        << "%\n";

    std::cout << "====================================\n";

    for (size_t s = 0; s < sheetLayouts.size(); s++)
    {
        const Sheet& sheet = sheetLayouts[s];

        std::cout << "\n--- Sheet " << (s + 1) << " ---\n";
        std::cout << "Placed Rects: " << sheet.placed.size() << "\n";

        for (size_t i = 0; i < sheet.placed.size(); i++)
        {
            const PlacedRect& pr = sheet.placed[i];

            std::cout
                << "  Rect " << (i + 1)
                << " | x=" << pr.x
                << " y=" << pr.y
                << " w=" << pr.width
                << " h=" << pr.height
                << " rot=" << (pr.rotated ? "Y" : "N")
                << "\n";
        }
    }
}