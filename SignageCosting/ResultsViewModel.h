#pragma once

#include "InvoiceViewModel.h"
#include <string>
#include <vector>

struct ResultsViewModel
{
    InvoiceViewModel invoice;

    int sheetsUsed = 0;
    bool nestingValid = true;
    std::vector<std::string> nestingErrors;
};
