#pragma once
#include <string>
#include <unordered_map>

struct ProductionRates
{
    double printPerM2 = 0.0;
    double laminatePerM2 = 0.0;
    double plotterCutPerM = 0.0;
    double routerCutPerM = 0.0;
    double applicationPerM2 = 0.0;
    double framePerM2 = 0.0;
};

class ProductionPricingDatabase
{
public:
    static void loadDefaults();

    static double getPrintRate();
    static double getLaminateRate();
    static double getPlotterCutRate();
    static double getRouterCutRate();
    static double getApplicationRate();
    static double getFrameRate();

private:
    static ProductionRates rates;
};
