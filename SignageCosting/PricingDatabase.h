#pragma once
#include <string>

//DEFINITION//
// ----------------- //
// This file defines:
// What pricing information is exposed to the rest of the program,
// and how it must be accessed safely.

class PricingDatabase
{
public:
    static void load(const std::string& jsonData);

    static double getLabourPerM2();
    static double getMarkupPercent();

    // UPDATED: now uses variant label matching instead of type + usage
    static double getMaterialCost(
        const std::string& materialId,
        const std::string& variantLabel);

    // Production rates (temporary but clean integration)
    static double getPrintRate();
    static double getLaminateRate();
    static double getPlotterCutRate();
    static double getRouterCutRate();
    static double getApplicationRate();
    static double getFrameRate();

private:
    static double labourPerM2;
    static double markupPercent;
};