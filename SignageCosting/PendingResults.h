#pragma once

#include "CostEngine.h"
#include "ResultsViewModel.h"

struct PendingResults
{
    CostResult result;

    ResultsViewModel viewModel;

    bool ready = false;
};
