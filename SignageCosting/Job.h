#pragma once
#include <vector>
#include <string>
#include "Material.h"
#include "ProductionOptions.h"
#include "Customer.h"

//DEFINITION//
// ----------------- //
// Shopping List for Signage - Makes list for Cost engine to calculate //
// It’s the thing that tells the system “what the user wants to make”, before the system goes off and tries to figure out how much it costs to ruin expensive material doing it.//

//Job = the whole order
//JobItem = one line on that order
//Everything inside = instructions like :
//what material
//how big
//how many
//which version of the material
//whether it’s roll or sheet nonsense

struct JobItem
{
    Material material;

    double width;
    double height;
    int quantity;

    double lengthMeters = 0.0;

    int variantIndex = 0;

    double selectedRollWidth = 0.0;

    SheetFormat selectedSheetFormat;

    bool autoRoll = true;

    ProductionOptions production;
};

class Job
{
public:

    Customer customer;

    std::vector<JobItem> items;

    void addItem(JobItem item);
};