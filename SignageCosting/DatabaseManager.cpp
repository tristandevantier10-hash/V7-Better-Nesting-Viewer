#include "DatabaseManager.h"

#include "FileSystem.h"
#include "HttpClient.h"
#include "MaterialDatabase.h"
#include "PricingDatabase.h"
#include "ProductionPricingDatabase.h"
#include "CustomerDatabase.h"

bool DatabaseManager::initialise()
{
    FileSystem::initialise();

    if (!loadMaterials())
        return false;

    if (!loadPricing())
        return false;

    if (!loadCustomers())
        return false;

    ProductionPricingDatabase::loadDefaults();

    return true;
}

bool DatabaseManager::loadMaterials()
{
    std::string materialsJson =
        HttpClient::get(
            "https://raw.githubusercontent.com/tristandevantier10-hash/Signage-Material-Costing/main/materials.json");

    if (materialsJson.empty())
        return false;

    MaterialDatabase::load(materialsJson);

    return true;
}

bool DatabaseManager::loadPricing()
{
    std::string pricingJson =
        HttpClient::get(
            "https://raw.githubusercontent.com/tristandevantier10-hash/Signage-Material-Costing/main/pricing.json");

    if (pricingJson.empty())
        return false;

    PricingDatabase::load(pricingJson);

    return true;
}

bool DatabaseManager::loadCustomers()
{
    CustomerDatabase::initialise();

    return true;
}