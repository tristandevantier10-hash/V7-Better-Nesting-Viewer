#include "InvoiceViewModelBuilder.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <chrono>
#include "JobSummaryViewModel.h"
#include <windows.h>

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

    // --- BULLETPROOF PC DATE PULL ---
    SYSTEMTIME st;
    GetLocalTime(&st); // Pulls directly from your Windows taskbar clock

    char dateBuffer[50];
    sprintf_s(dateBuffer, "%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);

    vm.invoiceDate = std::string(dateBuffer); // Saves your actual system date
    // ---------------------------------

    vm.customer = result.customer.company;

    vm.customerContact =
        result.customer.contact;

    vm.customerPhone =
        result.customer.phone;

    vm.customerEmail =
        result.customer.email;

    vm.materialCost = formatCurrency(result.materialCost);
    vm.labourCost = formatCurrency(result.labourCost);
    vm.productionCost = formatCurrency(result.productionCost);
    vm.subtotal = formatCurrency(result.totalCost);
    vm.markup = formatCurrency(result.margin);
    vm.sellPrice = formatCurrency(result.sellPrice);

    double vat =
        result.sellPrice * 0.15;

    double total =
        result.sellPrice + vat;

    vm.vat =
        formatCurrency(vat);

    vm.total =
        formatCurrency(total);

    if (!result.items.empty())
    {
        for (const auto& item : result.items)
        {

            std::cout << "Category = " << item.category << std::endl;

            JobSummaryViewModel job;

            job.material = item.materialId;

            job.variant = item.variant;

            job.quantity = std::to_string(item.quantity);

            job.sellPrice =
                formatCurrency(item.sellPrice);

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