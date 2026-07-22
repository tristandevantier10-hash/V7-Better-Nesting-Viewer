#include "InvoicePrinter.h"
#include "Format.h"
#include <iostream>
#include "ProductionPricingDatabase.h"
#include "Job.h"

void InvoicePrinter::print(const CostResult& result)
{
    std::cout << "\n==================================================\n";
    std::cout << "                INVOICE BREAKDOWN\n";
    std::cout << "==================================================\n";

    double runningTotal = 0.0;
    int jobIndex = 1;

    for (const auto& item : result.items)
    {
        runningTotal += item.sellPrice;

        std::cout << "\n" << jobIndex << ") "
            << item.materialId << " [" << item.category << "]\n";

        std::cout << "   Area: " << item.area << " m²\n";

        std::cout << "   Material: " << Format::money(item.materialCost)
            << " | Labour: " << Format::money(item.labourCost)
            << " | Prod: " << Format::money(item.productionCost)
            << " | Markup (100%): " << Format::money(item.markupValue) << "\n";

        // ---------------- PRODUCTION INLINE ----------------
        std::cout << "  -------------------------------------------------------------------------------------------- \n ";
        std::cout << "  Production:   ";

        bool first = true;

        auto printItem = [&](const std::string& name, double value)
            {
                if (!first) std::cout << " | ";
                first = false;
                std::cout << name << " " << Format::money(value);
            };

        if (item.production.print)
            printItem("Print", item.area * ProductionPricingDatabase::getPrintRate());

        if (item.production.laminate)
            printItem("Lam", item.area * ProductionPricingDatabase::getLaminateRate());

        if (item.production.plotterCut)
            printItem("Cut", item.quantity * ProductionPricingDatabase::getPlotterCutRate());

        if (item.production.routerCut)
            printItem("Router", item.quantity * ProductionPricingDatabase::getRouterCutRate());

        if (item.production.application)
            printItem("App", item.area * ProductionPricingDatabase::getApplicationRate());

        std::cout << "\n";
        std::cout << "  -------------------------------------------------------------------------------------------- \n ";
        std::cout << "   SELL PRICE: " << Format::money(item.sellPrice) << "\n\n";

        // ================= ROLL INPUT CHECK (INVOICE ONLY) =================
        if (item.category == "Roll")
        {
            const RollSolution& roll = item.rollSolution;

            std::cout << "\n   ROLL INPUT CHECK\n";
            std::cout << "   Item Size           : " << item.width << " x " << item.height << " mm\n";
            std::cout << "   Quantity            : " << item.quantity << "\n";
            std::cout << "   ---------------------------------\n";

            std::cout << "   BEST ROLL SELECTED\n";
            std::cout << "   Roll Width          : " << roll.rollWidth << " mm\n";
            std::cout << "   Mode                : " << (item.autoRoll ? "AUTO OPTIMIZER" : "MANUAL SELECTION") << "\n";

            std::cout << "   Efficiency          : " << roll.efficiency << " %\n";
            std::cout << "   Waste Area          : " << roll.wasteArea << " m²\n";
            std::cout << "   Length Used         : " << roll.lengthUsed << " m\n";
            std::cout << "   Rotated             : " << (roll.rotated ? "YES" : "NO") << "\n";

            std::cout << "   ---------------------------------\n\n";
        }

        jobIndex++;
    }

    // ---------------- FINAL SUMMARY ----------------
    std::cout << "==================================================\n";

    std::cout << " TOTAL COST = Job ";
    for (int i = 1; i < jobIndex; i++)
    {
        std::cout << i;
        if (i < jobIndex - 1)
            std::cout << " + ";
    }

    std::cout << " = " << Format::money(result.sellPrice) << "\n";

    std::cout << "==================================================\n";
}