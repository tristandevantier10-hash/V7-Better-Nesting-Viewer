#pragma once

#include "InvoiceViewModel.h"
#include "CostEngine.h"

class InvoiceViewModelBuilder
{
public:

    static InvoiceViewModel build(
        const CostResult& result);
};
