#pragma once

#include <string>
#include <vector>

#include "QuoteRecord.h"

class QuoteDatabase
{
public:
    static bool initialise();

    static bool save();

    static bool saveQuote(
        InvoiceViewModel& invoice,
        const std::string& status = "Draft");

    static const std::vector<QuoteRecord>& getAll();

    static bool findByReference(
        const std::string& reference,
        QuoteRecord& result);

    static bool updateStatus(
        const std::string& reference,
        const std::string& status);

    static int count();

private:
    static std::vector<QuoteRecord> quotes;

    static bool load();
};
