#pragma once
#include <vector>
#include "NestingCalculator.h"

struct RollSolution
{
    double rollWidth;
    bool rotated;
    double lengthUsed;
    double wasteArea;
    double efficiency;
};

class RollOptimizer
{
public:
    static RollSolution evaluate(
        double widthMM,
        double heightMM,
        int qty,
        const std::vector<double>& rollWidthsMM
    );
};
