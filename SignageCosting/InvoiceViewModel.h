#pragma once

#include <string>
#include <vector>
#include "JobSummaryViewModel.h"

struct InvoiceViewModel
{
    std::string invoiceDate;

    std::string jobref;

    std::string customer;

    std::string customerContact;

    std::string customerPhone;

    std::string customerEmail;

    std::string materialCost;

    std::string labourCost;

    std::string productionCost;

    std::string subtotal;

    std::string markup;

    std::string sellPrice;

    std::string vat;

    std::string total;

    std::vector<JobSummaryViewModel> jobs;
};
