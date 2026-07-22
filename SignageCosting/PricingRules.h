#pragma once

//DEFINITION//
// ----------------- //
//t calculates area (because humans keep forgetting to do it properly)
//It calculates how many sheets you need(by rounding up like a responsible adult)

class PricingRules
{
public:
    static double calculateArea(double width, double height); // Area for Vinyl
    static int calculateSheets(double jobArea, double sheetArea); // Sheets for Solid Substrates

};