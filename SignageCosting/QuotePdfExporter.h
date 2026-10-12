#pragma once

#include <string>
#include "InvoiceViewModel.h"

class QuotePdfExporter
{
public:
    static bool exportToPdf(
        const InvoiceViewModel& invoice,
        const std::string& outputPath,
        std::string& errorMessage);
};
