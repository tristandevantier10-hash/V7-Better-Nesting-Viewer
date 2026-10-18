#pragma once

#include <string>
#include <vector>

struct ProductionOptions
{
    // IDs of the company-configured processes applied to this job item.
    std::vector<std::string> selectedProcessIds;

    // Kept temporarily so existing quote and test-job data remains readable.
    bool print = false;
    bool laminate = false;
    bool plotterCut = false;
    bool routerCut = false;
    bool application = false;
    bool frame = false;
};
