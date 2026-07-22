#include <algorithm>
#include "NestingCalculator.h"
#include "iostream"

NestingResult NestingCalculator::Calculate(
    double widthMM,
    double heightMM,
    int quantity,
    double rollWidthMM)
{

    std::cout << "\n===== NESTING CALCULATOR =====\n";

    std::cout << "Input Width  : " << widthMM << "\n";
    std::cout << "Input Height : " << heightMM << "\n";
    std::cout << "Quantity     : " << quantity << "\n";
    std::cout << "Roll Width   : " << rollWidthMM << "\n";

    NestingResult result;

    // Convert mm to metres
    double widthM = widthMM / 1000.0;
    double heightM = heightMM / 1000.0;
    double rollWidthM = rollWidthMM / 1000.0;

    // Decide if we need to rotate the artwork to fit roll width
    if (widthM > rollWidthM && heightM <= rollWidthM)
    {
        std::swap(widthM, heightM);
        result.rotated = true;
    }
    else
    {
        result.rotated = false;
    }

    // How many pieces fit across roll width
    result.piecesAcross = static_cast<int>(rollWidthM / widthM);

    std::cout << "Pieces Across: "
        << result.piecesAcross
        << "\n";

    // Safety fallback
    if (result.piecesAcross < 1)
        result.piecesAcross = 1;

    // Calculate how many rows are needed (ceiling division)
    result.rows = (quantity + result.piecesAcross - 1) / result.piecesAcross;

    std::cout << "Rows: "
        << result.rows
        << "\n";

    result.requiredLengthM = result.rows * heightM;

    std::cout << "Required Length: "
        << result.requiredLengthM
        << " m\n";

    result.consumedAreaM2 = rollWidthM * result.requiredLengthM;

    double itemAreaM2 = (widthM * heightM) * quantity;

    result.theoreticalAreaM2 = itemAreaM2;

    // NEW: Waste calculation
    result.wasteAreaM2 = result.consumedAreaM2 - result.theoreticalAreaM2;

    // Avoid divide by zero
    if (result.consumedAreaM2 > 0)
    {
        result.efficiencyPercent = (itemAreaM2 / result.consumedAreaM2) * 100.0;
        result.wastePercent = 100.0 - result.efficiencyPercent;
    }
    else
    {
        result.efficiencyPercent = 0;
        result.wastePercent = 0;
    }

    return result;
}