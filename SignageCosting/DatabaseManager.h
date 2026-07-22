#pragma once

class DatabaseManager
{
public:

    static bool initialise();

private:

    static bool loadMaterials();

    static bool loadPricing();

    static bool loadCustomers();
};
