#include "PricingRules.h"
#include <cmath>

//DEFINITION//
// ----------------- //
//Turns millimetres into square metres so your costing doesn’t go off the rails
//Figures out how many full sheets you need, and refuses to pretend half a sheet is a real thing

// -----------------------------
// PURE MATH UTILITIES ONLY // 
// -----------------------------

double PricingRules::calculateArea(double width, double height)
{
    return (width / 1000.0) * (height / 1000.0);
}

int PricingRules::calculateSheets(double jobArea, double sheetArea)
{
    if (sheetArea <= 0.0)
        return 0;

    return static_cast<int>(std::ceil(jobArea / sheetArea));
}