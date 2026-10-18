#pragma once
#include <string>
#include <unordered_map>
#include <vector>

enum class ProcessRateUnit { PerSquareMetre, PerLinearMetre, PerItem, PerJob };

struct LabourProcess
{
    std::string id;
    std::string name;
    double rate = 0.0;
    ProcessRateUnit unit = ProcessRateUnit::PerItem;
    bool enabled = true;
};

struct ProductionRates
{
    double printPerM2 = 0.0;
    double laminatePerM2 = 0.0;
    double plotterCutPerM = 0.0;
    double routerCutPerM = 0.0;
    double applicationPerM2 = 0.0;
    double framePerM2 = 0.0;
};

class ProductionPricingDatabase
{
public:
    static void loadDefaults();
    static void loadProcesses();
    static bool saveProcesses();
    static const std::vector<LabourProcess>& getProcesses();
    static bool addProcess(const LabourProcess& process);
    static bool updateProcess(const LabourProcess& process);
    static double calculateProcessCost(const LabourProcess& process,
        double totalAreaM2, double totalLengthM, int totalItems);

    static double getPrintRate();
    static double getLaminateRate();
    static double getPlotterCutRate();
    static double getRouterCutRate();
    static double getApplicationRate();
    static double getFrameRate();

private:
    static ProductionRates rates;
    static std::vector<LabourProcess> processes;
};
