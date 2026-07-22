#pragma once

#include <string>
#include "Job.h"
#include "CostEngine.h"

class QuoteDocument
{
public:

    Job job;

    CostResult result;

    std::string customer;

    std::string reference;

    std::string notes;
};
