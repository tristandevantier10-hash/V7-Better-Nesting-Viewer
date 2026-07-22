#include "ProductionPricingDatabase.h"

ProductionRates ProductionPricingDatabase::rates;

void ProductionPricingDatabase::loadDefaults()
{
    rates.printPerM2 = 120.0;
    rates.laminatePerM2 = 80.0;
    rates.plotterCutPerM = 15.0;
    rates.routerCutPerM = 45.0;
    rates.applicationPerM2 = 90.0;
    rates.framePerM2 = 150.0;
}

double ProductionPricingDatabase::getPrintRate() { return rates.printPerM2; }
double ProductionPricingDatabase::getLaminateRate() { return rates.laminatePerM2; }
double ProductionPricingDatabase::getPlotterCutRate() { return rates.plotterCutPerM; }
double ProductionPricingDatabase::getRouterCutRate() { return rates.routerCutPerM; }
double ProductionPricingDatabase::getApplicationRate() { return rates.applicationPerM2; }
double ProductionPricingDatabase::getFrameRate() { return rates.framePerM2; }