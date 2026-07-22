#include "InvoiceViewModelBuilder.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include "JobSummaryViewModel.h"

namespace
{
    std::string formatCurrency(double value)
    {
        std::ostringstream ss;

        ss << "R "
            << std::fixed
            << std::setprecision(2)
            << value;

        return ss.str();
    }

#include <cmath>

    std::string formatNumber(double value)
    {
        std::ostringstream ss;

        if (std::fabs(value - std::round(value)) < 0.0001)
        {
            ss << static_cast<int>(std::round(value));
        }
        else
        {
            ss << std::fixed
                << std::setprecision(2)
                << value;
        }

        return ss.str();
    }
}

InvoiceViewModel
InvoiceViewModelBuilder::build(const CostResult& result)
{
    InvoiceViewModel vm;

    vm.materialCost = formatCurrency(result.materialCost);
    vm.labourCost = formatCurrency(result.labourCost);
    vm.productionCost = formatCurrency(result.productionCost);
    vm.subtotal = formatCurrency(result.totalCost);
    vm.markup = formatCurrency(result.margin);
    vm.sellPrice = formatCurrency(result.sellPrice);

    if (!result.items.empty())
    {
        for (const auto& item : result.items)
        {

            std::cout << "Category = " << item.category << std::endl;

            JobSummaryViewModel job;

            job.material = item.materialId;

            job.variant = item.variant;

            job.quantity = std::to_string(item.quantity);

            std::ostringstream ss;

            ss << std::fixed
                << std::setprecision(2)
                << item.area
                << " m²";

            job.area = ss.str();

            if (item.category == "Roll")
            {
                job.isRoll = true;

                ss.str("");
                ss.clear();

                ss << item.rollSolution.rollWidth
                    << " mm";
                job.rollWidth = ss.str();

                ss.str("");
                ss.clear();

                ss << std::fixed
                    << std::setprecision(2)
                    << item.rollSolution.lengthUsed
                    << " m";
                job.lengthUsed = ss.str();

                ss.str("");
                ss.clear();

                ss << std::fixed
                    << std::setprecision(2)
                    << item.rollSolution.efficiency
                    << " %";
                job.efficiency = ss.str();
            }
            else
            {
                job.isRoll = false;

                ss.str("");
                ss.clear();

                ss << formatNumber(item.width)
                    << " x "
                    << formatNumber(item.height)
                    << " mm";

                job.sheetSize = ss.str();

                ss.str("");
                ss.clear();

                ss << item.sheetsUsed;
                job.sheetsUsed = ss.str();
            }

            vm.jobs.push_back(job);
        }
    }

    std::cout << "\n===== INVOICE VM =====\n";
    std::cout << "Material   : " << vm.materialCost << "\n";
    std::cout << "Labour     : " << vm.labourCost << "\n";
    std::cout << "Production : " << vm.productionCost << "\n";
    std::cout << "Markup     : " << vm.markup << "\n";
    std::cout << "Sell Price : " << vm.sellPrice << "\n";

    return vm;
}