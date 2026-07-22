#pragma once

#include "NestingResult.h"

class NestingCalculator
{
public:

    static NestingResult Calculate(
        double widthMM,
        double heightMM,
        int quantity,
        double rollWidthMM);
};
