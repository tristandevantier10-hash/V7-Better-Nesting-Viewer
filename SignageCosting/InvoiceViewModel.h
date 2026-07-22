#pragma once

#include <string>
#include <vector>
#include "JobSummaryViewModel.h"

struct InvoiceViewModel
{
    std::string materialCost;

    std::string labourCost;

    std::string productionCost;

    std::string subtotal;

    std::string markup;

    std::string sellPrice;

    std::vector<JobSummaryViewModel> jobs;
};
