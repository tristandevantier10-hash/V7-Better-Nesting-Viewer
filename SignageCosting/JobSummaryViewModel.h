#pragma once

#include <string>
#include <vector>

struct JobSummaryViewModel
{
    std::string material;
    std::string variant;
    std::string quantity;
    std::string area;
    std::string sellPrice;
    std::string unitSize;
    std::string labourCost;
    std::vector<std::string> labourBreakdown;

    bool isRoll = true;

    // Roll
    std::string rollWidth;
    std::string lengthUsed;
    std::string efficiency;

    // Sheet
    std::string sheetSize;
    std::string sheetsUsed;
};
