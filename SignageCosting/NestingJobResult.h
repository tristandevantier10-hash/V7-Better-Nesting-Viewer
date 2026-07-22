#pragma once

#include <vector>

#include "NestGroup.h"
#include "NestingEngine.h"

struct NestingJobResult
{
    std::vector<NestGroup> groups;

    std::vector<Sheet> sheets;

    double wasteAreaM2 = 0.0;

    double efficiencyPercent = 0.0;
};
