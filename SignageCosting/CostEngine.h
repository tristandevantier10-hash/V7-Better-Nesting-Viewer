#pragma once
#include "Job.h"
#include <vector>
#include <string>
#include "ProductionOptions.h"
#include "RollOptimizer.h"
#include "NestingCore.h"
#include "NestGroup.h"

//DEFINITION//
// ----------------- //
//It defines what a cost result looks like
//It stores :
//what you made
//how big it was
//what it cost to make
//how much labour got involved
//Then it totals everything into a final price

struct CostTrace
{
    std::string materialId;
    std::string variantLabel;

    double width = 0;
    double height = 0;
    int qty = 0;

    double areaPerItem = 0;
    double totalArea = 0;

    double labourFactor = 1.0;
    double labourCost = 0;

    double materialCost = 0;
};

struct ItemCostResult
{
    // ---------------- CORE ----------------
    std::string materialId;
    std::string category;
    std::string variant;

    int quantity = 0;
    double area = 0.0;

    double materialCost = 0.0;
    double labourCost = 0.0;

    double markupValue = 0.0;
    double totalCost = 0.0;
    double sellPrice = 0.0;

    // ---------------- PRODUCTION ----------------
    ProductionOptions production;

    // ---------------- DEBUG ----------------
    double rollWidth = 0.0;
    double requiredMeters = 0.0;
    double rollRate = 0.0;

    double sheetsUsed = 0.0;
    double sheetRate = 0.0;

    double totalLength = 0.0;
    double productionCost = 0.0;

    RollSolution rollSolution;

    // ================= FIX: MIRROR INPUT DATA =================
    double width = 0.0;
    double height = 0.0;
    bool autoRoll = false;
    int variantIndex = 0;
};

struct CostResult
{
    Customer customer;

    std::vector<ItemCostResult> items;
    std::vector<Sheet> nestingSheets;

    double materialCost = 0.0;
    double labourCost = 0.0;

    double totalCost = 0.0;   // base cost (NO markup)
    double sellPrice = 0.0;   // final invoice total
    double margin = 0.0;
    double productionCost = 0.0;
};

class CostEngine
{
public:
    CostResult calculate(Job job);

private:

    void calculateRollItem(
        const JobItem& item,
        ItemCostResult& itemResult,
        CostResult& result,
        double totalArea,
        double& materialCost);

    void calculateSheetItem(
        const JobItem& item,
        const NestGroup& group,
        ItemCostResult& itemResult,
        CostResult& result,
        double totalArea,
        double& materialCost);

    void calculateGenericItem(
        const JobItem& item,
        double totalArea,
        double& materialCost);
};

