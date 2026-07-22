#pragma once

#include <string>

struct JobSummaryViewModel
{
    std::string material;
    std::string variant;
    std::string quantity;
    std::string area;

    bool isRoll = true;

    // Roll
    std::string rollWidth;
    std::string lengthUsed;
    std::string efficiency;

    // Sheet
    std::string sheetSize;
    std::string sheetsUsed;
};
