#pragma once

#include <vector>
#include "Job.h"
#include "NestGroup.h"
#include "NestingEngine.h"
#include "NestingJobResult.h"

class NestingManager
{
public:

    std::vector<NestGroup> buildGroups(
        const Job& job);

    NestingJobResult calculate(
        const Job& job);
};
