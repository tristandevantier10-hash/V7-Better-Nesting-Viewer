#pragma once

#include <string>
#include <vector>
#include "NestingEngine.h"

struct NestGroup
{
    std::string materialId;
    std::string variant;

    int sheetWidth = 0;
    int sheetHeight = 0;

    int rollWidth = 0;

    bool isRoll = false;

    std::vector<Rect> rects;

    std::vector<int> jobItemIndices;

    std::vector<Sheet> layouts;

    // NEW
    int sheetCount = 0;

    double rollLengthM = 0.0;

    double wasteAreaM2 = 0.0;

    double efficiencyPercent = 0.0;

    double materialCost = 0.0;
};
