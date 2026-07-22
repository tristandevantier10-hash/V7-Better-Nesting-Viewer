#include "TestJobFactory.h"
#include "MaterialDatabase.h"
#include <iostream>

static void applyDefaultProduction(JobItem& item)
{
    item.production.print = true;
    item.production.laminate = true;
    item.production.plotterCut = true;
    item.production.routerCut = true;
    item.production.application = true;
    item.production.frame = false;
}

Job TestJobFactory::createDefaultTestJob()
{
    Job job;


    // =====================================================
    // VINYL TEST
    // Roll optimizer should remain untouched
    // =====================================================

    JobItem vinylItem;

    vinylItem.material = MaterialDatabase::get("VINYL");
    vinylItem.width = 300;
    vinylItem.height = 600;
    vinylItem.quantity = 5;
    vinylItem.variantIndex = 4;

    const auto& vinylVariant =
        vinylItem.material.variants[vinylItem.variantIndex];

    vinylItem.selectedRollWidth =
        vinylVariant.roll_widths.empty()
        ? 0
        : vinylVariant.roll_widths[0];

    applyDefaultProduction(vinylItem);

    job.addItem(vinylItem);



    // =====================================================
    // CORE TEST 1
    // Basic current working example
    // Expected:
    // 5 pieces
    // 1 sheet
    // =====================================================

    JobItem coreBasic;

    coreBasic.material = MaterialDatabase::get("CHROMADEK");
    coreBasic.width = 600;
    coreBasic.height = 1300;
    coreBasic.quantity = 5;
    coreBasic.variantIndex = 2;

    applyDefaultProduction(coreBasic);

    job.addItem(coreBasic);



    // =====================================================
    // CORE TEST 2
    // Multiple rows
    //
    // Sheet 2440x1220
    //
    // 8 x 600mm wide pieces
    // should force row movement
    // =====================================================

    JobItem coreRows;

    coreRows.material = MaterialDatabase::get("CHROMADEK");
    coreRows.width = 600;
    coreRows.height = 600;
    coreRows.quantity = 8;
    coreRows.variantIndex = 2;

    applyDefaultProduction(coreRows);

    job.addItem(coreRows);



    // =====================================================
    // CORE TEST 3
    // Multiple sheets
    //
    // Large quantity
    // =====================================================

    JobItem coreMultiSheet;

    coreMultiSheet.material = MaterialDatabase::get("CHROMADEK");
    coreMultiSheet.width = 800;
    coreMultiSheet.height = 800;
    coreMultiSheet.quantity = 10;
    coreMultiSheet.variantIndex = 2;

    applyDefaultProduction(coreMultiSheet);

    job.addItem(coreMultiSheet);



    // =====================================================
    // CORE TEST 4
    // Rotation test
    //
    // 1000x600
    // should rotate where needed
    // =====================================================

    JobItem coreRotation;

    coreRotation.material = MaterialDatabase::get("CHROMADEK");
    coreRotation.width = 1000;
    coreRotation.height = 600;
    coreRotation.quantity = 6;
    coreRotation.variantIndex = 2;

    applyDefaultProduction(coreRotation);

    job.addItem(coreRotation);



    // =====================================================
    // CORE TEST 5
    // Stress test
    //
    // Many small parts
    // =====================================================

    JobItem coreStress;

    coreStress.material = MaterialDatabase::get("CHROMADEK");
    coreStress.width = 200;
    coreStress.height = 300;
    coreStress.quantity = 50;
    coreStress.variantIndex = 2;

    applyDefaultProduction(coreStress);

    job.addItem(coreStress);



    return job;
}