#pragma once

#include "InvoiceViewModel.h"
#include "NestingEngine.h"
#include <string>
#include <vector>

struct ResultsViewModel
{
    InvoiceViewModel invoice;

    int sheetsUsed = 0;
    std::vector<Rect> nestingItems;
    bool nestingValid = true;
    std::vector<std::string> nestingErrors;
};
