#pragma once

#include <functional>
#include <string>

class DatabaseManager
{
public:

    using ProgressCallback =
        std::function<void(int)>;

    static bool initialise(
        ProgressCallback progressCallback = nullptr,
        std::string* errorMessage = nullptr);

private:

    static bool loadMaterials(std::string& errorMessage);
    static bool loadPricing(std::string& errorMessage);
    static bool loadCustomers(std::string& errorMessage);
};
