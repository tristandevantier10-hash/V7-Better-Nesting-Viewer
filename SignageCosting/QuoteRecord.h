#pragma once

#include <string>
#include "InvoiceViewModel.h"

struct QuoteRecord
{
    InvoiceViewModel invoice;

    // Draft, Sent, Accepted, Declined or Expired
    std::string status = "Draft";

    // Date and time the quote was saved
    std::string savedAt;

    // Date and time the quote status last changed
    std::string statusUpdatedAt;
};
