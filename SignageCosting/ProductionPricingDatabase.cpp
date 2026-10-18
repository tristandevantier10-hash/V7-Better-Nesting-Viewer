#include "ProductionPricingDatabase.h"
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>

ProductionRates ProductionPricingDatabase::rates;
std::vector<LabourProcess> ProductionPricingDatabase::processes;
using ProcessJson = nlohmann::json;

void ProductionPricingDatabase::loadDefaults()
{
    rates.printPerM2 = 120.0;
    rates.laminatePerM2 = 80.0;
    rates.plotterCutPerM = 15.0;
    rates.routerCutPerM = 45.0;
    rates.applicationPerM2 = 90.0;
    rates.framePerM2 = 150.0;

    if (processes.empty())
        loadProcesses();
}

void ProductionPricingDatabase::loadProcesses()
{
    const std::vector<LabourProcess> defaults = {
        { "print", "Print", 120.0, ProcessRateUnit::PerSquareMetre, true },
        { "laminate", "Laminate", 80.0, ProcessRateUnit::PerSquareMetre, true },
        { "plotter-cut", "Plotter cut", 15.0, ProcessRateUnit::PerLinearMetre, true },
        { "router-cut", "Router cut", 45.0, ProcessRateUnit::PerLinearMetre, true },
        { "application", "Apply to substrate", 90.0, ProcessRateUnit::PerSquareMetre, true },
        { "frame", "Frame", 150.0, ProcessRateUnit::PerSquareMetre, true }
    };

    std::ifstream input("Data/labour_processes.json");
    if (!input)
    {
        processes = defaults;
        saveProcesses();
        return;
    }

    try
    {
        const ProcessJson document = ProcessJson::parse(input);
        processes.clear();
        for (const auto& entry : document)
        {
            const std::string unit = entry.value("unit", std::string("item"));
            ProcessRateUnit rateUnit = ProcessRateUnit::PerItem;
            if (unit == "area") rateUnit = ProcessRateUnit::PerSquareMetre;
            else if (unit == "length") rateUnit = ProcessRateUnit::PerLinearMetre;
            else if (unit == "job") rateUnit = ProcessRateUnit::PerJob;
            processes.push_back({
                entry.value("id", std::string{}),
                entry.value("name", std::string{}),
                entry.value("rate", 0.0), rateUnit,
                entry.value("enabled", true) });
        }
        if (processes.empty()) processes = defaults;
    }
    catch (const ProcessJson::exception&)
    {
        processes = defaults;
    }
}

bool ProductionPricingDatabase::saveProcesses()
{
    ProcessJson document = ProcessJson::array();
    for (const auto& process : processes)
    {
        const char* unit = "item";
        if (process.unit == ProcessRateUnit::PerSquareMetre) unit = "area";
        else if (process.unit == ProcessRateUnit::PerLinearMetre) unit = "length";
        else if (process.unit == ProcessRateUnit::PerJob) unit = "job";
        document.push_back({ {"id", process.id}, {"name", process.name},
            {"rate", process.rate}, {"unit", unit}, {"enabled", process.enabled} });
    }
    std::ofstream output("Data/labour_processes.json");
    if (!output) return false;
    output << document.dump(2);
    return static_cast<bool>(output);
}

const std::vector<LabourProcess>& ProductionPricingDatabase::getProcesses()
{
    return processes;
}

bool ProductionPricingDatabase::addProcess(const LabourProcess& process)
{
    if (process.id.empty() || process.name.empty() || process.rate < 0.0 ||
        std::any_of(processes.begin(), processes.end(), [&](const LabourProcess& item)
        { return item.id == process.id; })) return false;
    processes.push_back(process);
    return saveProcesses();
}

bool ProductionPricingDatabase::updateProcess(const LabourProcess& process)
{
    auto found = std::find_if(processes.begin(), processes.end(),
        [&](const LabourProcess& item) { return item.id == process.id; });
    if (found == processes.end() || process.name.empty() || process.rate < 0.0)
        return false;
    *found = process;
    return saveProcesses();
}

double ProductionPricingDatabase::calculateProcessCost(
    const LabourProcess& process, double areaM2, double lengthM, int items)
{
    double quantity = static_cast<double>(items);
    if (process.unit == ProcessRateUnit::PerSquareMetre) quantity = areaM2;
    else if (process.unit == ProcessRateUnit::PerLinearMetre) quantity = lengthM;
    else if (process.unit == ProcessRateUnit::PerJob) quantity = 1.0;
    return process.rate * quantity;
}

double ProductionPricingDatabase::getPrintRate() { return rates.printPerM2; }
double ProductionPricingDatabase::getLaminateRate() { return rates.laminatePerM2; }
double ProductionPricingDatabase::getPlotterCutRate() { return rates.plotterCutPerM; }
double ProductionPricingDatabase::getRouterCutRate() { return rates.routerCutPerM; }
double ProductionPricingDatabase::getApplicationRate() { return rates.applicationPerM2; }
double ProductionPricingDatabase::getFrameRate() { return rates.framePerM2; }
