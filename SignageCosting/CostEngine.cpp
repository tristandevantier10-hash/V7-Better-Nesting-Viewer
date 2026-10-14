#include "CostEngine.h"
#include "PricingRules.h"
#include "PricingDatabase.h"
#include <iostream>
#include <algorithm>
#include "RollOptimizer.h"
#include "ProductionPricingDatabase.h"
#include "NestGroup.h"
#include "NestingManager.h"

// ---------------- MAIN ENGINE ----------------

CostResult CostEngine::calculate(Job job)
{
    CostResult result;

    result.customer = job.customer;

    NestingManager nestingManager;

    auto nestingResult =
        nestingManager.calculate(job);

    result.nestingSheets = nestingResult.sheets;

    result.materialCost = 0.0;
    result.labourCost = 0.0;
    result.productionCost = 0.0;
    result.sellPrice = 0.0;

    const double labourRate = PricingDatabase::getLabourPerM2();
    const double markup = PricingDatabase::getMarkupPercent();

    for (size_t i = 0; i < job.items.size(); i++)
    {
        auto& item = job.items[i];
        const Material& m = item.material;

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

        if (rule == "ROLL_AREA")
        {
            calculateRollItem(
                item,
                itemResult,
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
    double totalArea,
    double& materialCost)
{
    const Material& material = item.material;
    const MaterialVariant& variant = material.variants[item.variantIndex];

    std::vector<double> rollOptions;
    if (item.autoRoll)
    {
        rollOptions.assign(variant.roll_widths.begin(), variant.roll_widths.end());
    }
    else
    {
        rollOptions.push_back(item.selectedRollWidth);
    }

    RollSolution solution = RollOptimizer::evaluate(
        item.width,
        item.height,
        item.quantity,
        rollOptions);

    const double requiredMeters = solution.lengthUsed;
    const double usedArea = (solution.rollWidth / 1000.0) * requiredMeters;
    solution.wasteArea = usedArea - totalArea;
    solution.efficiency = (totalArea / usedArea) * 100.0;

    itemResult.rollSolution = solution;
    itemResult.rollWidth = solution.rollWidth;

    materialCost = requiredMeters *
        PricingDatabase::getMaterialCost(material.id, variant.label);
}

void CostEngine::calculateSheetItem(
    const JobItem& item,
    const NestGroup& group,
    ItemCostResult& itemResult,
    double& materialCost)
{
    const Material& material = item.material;
    const MaterialVariant& variant = material.variants[item.variantIndex];
    const int sheetCount = static_cast<int>(group.layouts.size());

    itemResult.sheetsUsed = sheetCount;
    itemResult.width = item.selectedSheetFormat.width;
    itemResult.height = item.selectedSheetFormat.height;

    const double unitCost =
        PricingDatabase::getMaterialCost(material.id, variant.label);
    materialCost = sheetCount * unitCost;
}
