#include "NestingManager.h"
#include "MaterialDatabase.h"
#include "NestingCore.h"
#include "iostream"

std::vector<NestGroup> NestingManager::buildGroups(const Job& job)
{
    std::vector<NestGroup> groups;

    for (size_t i = 0; i < job.items.size(); i++)
    {
        const auto& item = job.items[i];

        const Material& m = item.material;

        const MaterialVariant& v =
            m.variants[item.variantIndex];

        // ------------------------------------
        // BUILD NEST GROUPS
        // ------------------------------------

        NestGroup* group = nullptr;

        for (auto& g : groups)
        {
            if (g.materialId == m.id &&
                g.variant == v.label &&
                g.isRoll == (m.cost_model == "ROLL_AREA"))
            {
                if (g.isRoll)
                {
                    if (g.rollWidth == item.selectedRollWidth)
                    {
                        group = &g;
                        break;
                    }
                }
                else
                {
                    if (g.sheetWidth == item.selectedSheetFormat.width &&
                        g.sheetHeight == item.selectedSheetFormat.height)
                    {
                        group = &g;
                        break;
                    }
                }
            }
        }

        if (!group)
        {
            NestGroup newGroup;

            newGroup.materialId = m.id;
            newGroup.variant = v.label;

            newGroup.isRoll = (m.cost_model == "ROLL_AREA");

            newGroup.rollWidth = item.selectedRollWidth;

            newGroup.sheetWidth = item.selectedSheetFormat.width;
            newGroup.sheetHeight = item.selectedSheetFormat.height;

            groups.push_back(newGroup);

            group = &groups.back();
        }

        group->rects.push_back(
            {
                item.width,
                item.height,
                item.quantity
            });

        group->jobItemIndices.push_back(static_cast<int>(i));
    }

    return groups;
}

NestingJobResult NestingManager::calculate(
    const Job& job)
{
    NestingJobResult result;

    result.groups = buildGroups(job);

    NestingCore core;

    for (auto& group : result.groups)
    {
        if (group.isRoll)
        {
            std::vector<PlacementStrategy> strategies =
            {
                PlacementStrategy::BestShortSideFit,
                PlacementStrategy::BestLongSideFit,
                PlacementStrategy::BestAreaFit,
                PlacementStrategy::BottomLeft
            };

            std::vector<Sheet> bestLayouts;
            double bestEfficiency = -1.0;
            double bestUsedLength = 0.0;

            constexpr double VIRTUAL_ROLL_LENGTH = 100000.0;

            for (auto strategy : strategies)
            {
                auto layouts =
                    core.pack(
                        group.rects,
                        group.rollWidth,
                        VIRTUAL_ROLL_LENGTH,
                        strategy);

                // Measure actual roll length used
                double usedLength = 0.0;

                for (auto& sheet : layouts)
                {
                    for (const auto& p : sheet.placed)
                    {
                        usedLength = std::max(
                            usedLength,
                            p.y + p.height);
                    }

                    // IMPORTANT: trim the virtual sheet
                    sheet.height = usedLength;
                }

                double usedArea = 0.0;

                for (const auto& r : group.rects)
                    usedArea += r.width * r.height * r.quantity;

                double totalArea = group.rollWidth * usedLength;

                double efficiency =
                    totalArea > 0.0 ?
                    (usedArea / totalArea) * 100.0 :
                    0.0;

                std::cout
                    << "Roll Strategy "
                    << static_cast<int>(strategy)
                    << " -> "
                    << efficiency
                    << "%\n";

                if (efficiency > bestEfficiency)
                {
                    bestEfficiency = efficiency;
                    bestLayouts = layouts;
                    bestUsedLength = usedLength;
                }
            }

            group.layouts = bestLayouts;

            result.sheets.insert(
                result.sheets.end(),
                bestLayouts.begin(),
                bestLayouts.end());

            group.sheetCount = 1;

            double totalObjectArea = 0.0;

            for (const auto& r : group.rects)
                totalObjectArea +=
                (r.width * r.height * r.quantity) / 1000000.0;

            double rollArea =
                (group.rollWidth * bestUsedLength) / 1000000.0;

            group.wasteAreaM2 = rollArea - totalObjectArea;

            group.efficiencyPercent =
                rollArea > 0.0 ?
                (totalObjectArea / rollArea) * 100.0 :
                0.0;

            continue;
        }

        std::vector<PlacementStrategy> strategies =
        {
            PlacementStrategy::BestShortSideFit,
            PlacementStrategy::BestLongSideFit,
            PlacementStrategy::BestAreaFit,
            PlacementStrategy::BottomLeft
        };

        std::vector<Sheet> bestLayouts;

        double bestEfficiency = -1.0;

        for (auto strategy : strategies)
        {
            auto layouts =
                core.pack(
                    group.rects,
                    group.sheetWidth,
                    group.sheetHeight,
                    strategy);

            double sheetArea =
                group.sheetWidth * group.sheetHeight;

            double usedArea = 0.0;

            for (const auto& r : group.rects)
                usedArea += r.width * r.height * r.quantity;

            double totalArea = sheetArea * layouts.size();

            double efficiency =
                (usedArea / totalArea) * 100.0;

            std::cout
                << "Strategy "
                << static_cast<int>(strategy)
                << " -> "
                << efficiency
                << "% using "
                << layouts.size()
                << " sheets\n";

            if (efficiency > bestEfficiency)
            {
                bestEfficiency = efficiency;
                bestLayouts = layouts;
            }
        }

        group.layouts = bestLayouts;

        result.sheets.insert(
            result.sheets.end(),
            bestLayouts.begin(),
            bestLayouts.end());

        // -----------------------------
        // GROUP STATISTICS
        // -----------------------------
        group.sheetCount =
            static_cast<int>(group.layouts.size());

        double totalObjectArea = 0.0;

        for (const auto& r : group.rects)
        {
            totalObjectArea +=
                (r.width * r.height * r.quantity) / 1000000.0;
        }

        double sheetArea =
            (group.sheetWidth * group.sheetHeight) / 1000000.0;

        double totalSheetArea =
            sheetArea * group.sheetCount;

        if (totalSheetArea > 0.0)
        {
            group.wasteAreaM2 =
                totalSheetArea - totalObjectArea;

            group.efficiencyPercent =
                (totalObjectArea / totalSheetArea) * 100.0;
        }

    }

    return result;
}