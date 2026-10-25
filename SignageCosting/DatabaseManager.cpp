#include "DatabaseManager.h"

#include "FileSystem.h"
#include "HttpClient.h"
#include "MaterialDatabase.h"
#include "PricingDatabase.h"
#include "ProductionPricingDatabase.h"
#include "CustomerDatabase.h"
#include "QuoteDatabase.h"

#include <filesystem>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>

namespace
{
    bool readCachedData(const std::string& filename, std::string& contents)
    {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open())
            return false;

        contents.assign(
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>());
        return !contents.empty() && !file.bad();
    }

    void saveCachedData(const std::string& filename, const std::string& contents)
    {
        const std::string temporaryFilename = filename + ".tmp";
        {
            std::ofstream file(temporaryFilename, std::ios::binary | std::ios::trunc);
            if (!file.is_open())
                return;

            file.write(contents.data(), static_cast<std::streamsize>(contents.size()));
            if (!file.good())
                return;
        }

        std::error_code error;
        std::filesystem::copy_file(
            temporaryFilename,
            filename,
            std::filesystem::copy_options::overwrite_existing,
            error);

        std::error_code cleanupError;
        std::filesystem::remove(temporaryFilename, cleanupError);
    }

    bool loadMaterialsData(const std::string& contents)
    {
        try
        {
            return MaterialDatabase::load(contents);
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    bool loadPricingData(const std::string& contents)
    {
        try
        {
            return PricingDatabase::load(contents);
        }
        catch (const std::exception&)
        {
            return false;
        }
    }
}

bool DatabaseManager::initialise(
    ProgressCallback progressCallback,
    std::string* errorMessage)
{
    std::string localError;
    std::string& error = errorMessage ? *errorMessage : localError;
    error.clear();
    FileSystem::initialise();

    if (progressCallback)
        progressCallback(0);

    if (!loadMaterials(error))
        return false;

    if (progressCallback)
        progressCallback(1);

    if (!loadPricing(error))
        return false;

    if (progressCallback)
        progressCallback(2);

    if (!loadCustomers(error))
        return false;

    if (progressCallback)
        progressCallback(3);

    ProductionPricingDatabase::loadDefaults();

    if (progressCallback)
        progressCallback(4);

    return true;
}

bool DatabaseManager::loadMaterials(std::string& errorMessage)
{
    std::string requestError;
    std::string materialsJson =
        HttpClient::get(
            "https://raw.githubusercontent.com/tristandevantier10-hash/Signage-Material-Costing/main/materials.json",
            requestError);

    std::string remoteFailure;
    if (materialsJson.empty())
    {
        remoteFailure = "The materials download failed: " + requestError + ".";
    }
    else if (loadMaterialsData(materialsJson))
    {
        saveCachedData(FileSystem::getDataFolder() + "materials.json", materialsJson);
        return true;
    }

    if (remoteFailure.empty())
        remoteFailure = "The materials data from the server is invalid or contains no materials.";

    std::string cachedMaterials;
    if (readCachedData(FileSystem::getDataFolder() + "materials.json", cachedMaterials) &&
        loadMaterialsData(cachedMaterials))
    {
        std::cerr << "Using cached materials data because the latest download was unavailable or invalid.\n";
        return true;
    }

    errorMessage = remoteFailure + " No valid cached materials data is available.";
    return false;
}

bool DatabaseManager::loadPricing(std::string& errorMessage)
{
    std::string requestError;
    std::string pricingJson =
        HttpClient::get(
            "https://raw.githubusercontent.com/tristandevantier10-hash/Signage-Material-Costing/main/pricing.json",
            requestError);

    std::string remoteFailure;
    if (pricingJson.empty())
    {
        remoteFailure = "The pricing download failed: " + requestError + ".";
    }
    else if (loadPricingData(pricingJson))
    {
        saveCachedData(FileSystem::getDataFolder() + "pricing.json", pricingJson);
        return true;
    }

    if (remoteFailure.empty())
        remoteFailure = "The pricing data from the server is invalid or incomplete.";

    std::string cachedPricing;
    if (readCachedData(FileSystem::getDataFolder() + "pricing.json", cachedPricing) &&
        loadPricingData(cachedPricing))
    {
        std::cerr << "Using cached pricing data because the latest download was unavailable or invalid.\n";
        return true;
    }

    errorMessage = remoteFailure + " No valid cached pricing data is available.";
    return false;
}

bool DatabaseManager::loadCustomers(std::string& errorMessage)
{
    CustomerDatabase::initialise();

    if (!QuoteDatabase::initialise())
    {
        errorMessage = "Could not open or read local quote history in Quotes/quotes.json. "
            "Check that the file is valid and that the application can write to its data folder.";
        return false;
    }

    return true;
}
