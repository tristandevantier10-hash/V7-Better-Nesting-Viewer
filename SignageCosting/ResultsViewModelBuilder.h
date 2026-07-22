#pragma once

#include "ResultsViewModel.h"
#include "CostEngine.h"

class ResultsViewModelBuilder
{
public:

    static ResultsViewModel build(
        const CostResult& result);
};