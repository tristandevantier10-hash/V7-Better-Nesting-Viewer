#pragma once

#include <functional>

class DatabaseManager
{
public:

    using ProgressCallback =
        std::function<void(int)>;

    static bool initialise(
        ProgressCallback progressCallback = nullptr);

private:

    static bool loadMaterials();
    static bool loadPricing();
    static bool loadCustomers();
};