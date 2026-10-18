#pragma once

#include <vector>
#include <string>

#include "NestGroup.h"
#include "NestingEngine.h"

struct NestingJobResult
{
    std::vector<NestGroup> groups;

    std::vector<Sheet> sheets;

    double wasteAreaM2 = 0.0;

    double efficiencyPercent = 0.0;
    bool valid = true;
    std::vector<std::string> errors;
};
