#pragma once

#include <string>
#include <vector>
#include "QuoteRecord.h"

class QuoteReferenceGenerator
{
public:
    static std::string generate(
        const std::vector<QuoteRecord>& quotes);
};
