#include "RollOptimizer.h"
#include <algorithm>
#include <iostream>

RollSolution RollOptimizer::evaluate(
    double widthMM,
    double heightMM,
    int qty,
    const std::vector<double>& rollWidthsMM)
{
    RollSolution best{};
    best.efficiency = -1;

    for (double roll : rollWidthsMM)
    {
        NestingResult n = NestingCalculator::Calculate(
            widthMM,
            heightMM,
            qty,
            roll
        );

        std::cout << "\n===== ROLL OPTION =====\n";
        std::cout << "Roll: " << roll << "\n";
        std::cout << "Length: " << n.requiredLengthM << "\n";
        std::cout << "Efficiency: " << n.efficiencyPercent << "\n";

        double eff = n.efficiencyPercent;

        if (eff > best.efficiency)
        {
            best.rollWidth = roll;
            best.rotated = n.rotated;
            best.lengthUsed = n.requiredLengthM;
            best.wasteArea = n.wasteAreaM2;
            best.efficiency = eff;
        }
    }

    return best;
}