#include "NestingManager.h"
#include "MaterialDatabase.h"
#include "NestingCore.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <utility>

namespace
{
    const std::array<PlacementStrategy, 5> kStrategies =
    {
        PlacementStrategy::BestShortSideFit,
        PlacementStrategy::BestLongSideFit,
        PlacementStrategy::BestAreaFit,
        PlacementStrategy::BottomLeft,
        PlacementStrategy::BestContactPoint
    };

    const std::array<ItemOrder, 12> kItemOrders =
    {
        ItemOrder::AreaDescending,
        ItemOrder::LongSideDescending,
        ItemOrder::ShortSideDescending,
        ItemOrder::WidthDescending,
        ItemOrder::HeightDescending,
        ItemOrder::PerimeterDescending,
        ItemOrder::AreaAscending,
        ItemOrder::LongSideAscending,
        ItemOrder::ShortSideAscending,
        ItemOrder::WidthAscending,
        ItemOrder::HeightAscending,
        ItemOrder::PerimeterAscending
    };


    size_t placedCount(const NestingPackResult& result)
    {
        size_t count = 0;
        for (const auto& sheet : result.sheets)
            count += sheet.placed.size();
        return count;
    }

    double occupiedEnvelopeArea(const NestingPackResult& result)
    {
        double total = 0.0;
        for (const auto& sheet : result.sheets)
        {
            double right = 0.0;
            double bottom = 0.0;
            for (const auto& part : sheet.placed)
            {
                right = std::max(right, part.x + part.width);
                bottom = std::max(bottom, part.y + part.height);
            }
            total += right * bottom;
        }
        return total;
    }

    bool betterSheetLayout(
        const NestingPackResult& candidate,
        const NestingPackResult& current,
        size_t candidateCapacity,
        size_t currentCapacity,
        double sheetArea,
        double partArea)
    {
        const bool candidateComplete = candidate.isValid();
        const bool currentComplete = current.isValid();
        if (candidateComplete != currentComplete)
            return candidateComplete;

        if (!candidateComplete)
        {
            if (placedCount(candidate) != placedCount(current))
                return placedCount(candidate) > placedCount(current);
            if (candidate.sheets.size() != current.sheets.size())
                return candidate.sheets.size() < current.sheets.size();
            if (candidateCapacity != currentCapacity)
                return candidateCapacity > currentCapacity;
            return candidate.validationErrors.size() < current.validationErrors.size();
        }

        if (candidate.sheets.size() != current.sheets.size())
            return candidate.sheets.size() < current.sheets.size();

        if (candidateCapacity != currentCapacity)
            return candidateCapacity > currentCapacity;

        const double candidateWaste = sheetArea * candidate.sheets.size() - partArea;
        const double currentWaste = sheetArea * current.sheets.size() - partArea;
        if (std::abs(candidateWaste - currentWaste) > 1e-7)
            return candidateWaste < currentWaste;

        return occupiedEnvelopeArea(candidate) + 1e-7 < occupiedEnvelopeArea(current);
    }

    bool betterRollLayout(
        const NestingPackResult& candidate,
        double candidateLength,
        const NestingPackResult& current,
        double currentLength,
        double rollWidth,
        double partArea)
    {
        const bool candidateComplete = candidate.isValid();
        const bool currentComplete = current.isValid();
        if (candidateComplete != currentComplete)
            return candidateComplete;

        if (!candidateComplete)
        {
            if (placedCount(candidate) != placedCount(current))
                return placedCount(candidate) > placedCount(current);
            return candidateLength + 1e-7 < currentLength;
        }

        if (std::abs(candidateLength - currentLength) > 1e-7)
            return candidateLength < currentLength;

        const double candidateWaste = rollWidth * candidateLength - partArea;
        const double currentWaste = rollWidth * currentLength - partArea;
        return candidateWaste + 1e-7 < currentWaste;
    }
}

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
            NestingPackResult bestPack;
            double bestUsedLength = 0.0;
            bool haveBest = false;
            double partArea = 0.0;
            for (const auto& item : group.rects)
                partArea += item.width * item.height * item.quantity;

            constexpr double VIRTUAL_ROLL_LENGTH = 100000.0;

            for (auto order : kItemOrders)
            {
                for (auto strategy : kStrategies)
                {
                    auto packed = core.pack(group.rects, group.rollWidth,
                        VIRTUAL_ROLL_LENGTH, strategy, order);

                    // Measure actual roll length used.
                    double usedLength = 0.0;
                    for (const auto& sheet : packed.sheets)
                        for (const auto& p : sheet.placed)
                            usedLength = std::max(usedLength, p.y + p.height);

                    for (auto& sheet : packed.sheets)
                        sheet.height = usedLength;

                    auto rollErrors = NestingCore::validateLayout(
                        packed.sheets, group.rollWidth, usedLength);
                    packed.validationErrors.insert(packed.validationErrors.end(),
                        rollErrors.begin(), rollErrors.end());

                    if (!haveBest || betterRollLayout(packed, usedLength,
                        bestPack, bestUsedLength, group.rollWidth, partArea))
                    {
                        bestPack = std::move(packed);
                        bestUsedLength = usedLength;
                        haveBest = true;
                    }
                }
            }


            group.layouts = bestPack.sheets;
            group.unplaced = bestPack.unplaced;
            group.nestingErrors = bestPack.validationErrors;
            if (!bestPack.isValid())
            {
                result.valid = false;
                result.errors.insert(result.errors.end(), group.nestingErrors.begin(), group.nestingErrors.end());
                for (const auto& item : group.unplaced)
                    result.errors.push_back(item.reason + " (" + std::to_string(item.quantity) + " unplaced)");
            }

            result.sheets.insert(
                result.sheets.end(),
                group.layouts.begin(),
                group.layouts.end());

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

        NestingPackResult bestPack;
        bool haveBest = false;
        size_t bestFitScore = 0;
        const double sheetAreaMM2 = group.sheetWidth * group.sheetHeight;
        double partArea = 0.0;
        for (const auto& item : group.rects)
            partArea += item.width * item.height * item.quantity;

        for (auto order : kItemOrders)
        {
            for (auto strategy : kStrategies)
            {
                auto packed = core.pack(group.rects, group.sheetWidth,
                    group.sheetHeight, strategy, order);
                const size_t packedCapacity = core.maxAdditionalFit(packed, group.rects);
                if (!haveBest || betterSheetLayout(packed, bestPack,
                    packedCapacity, bestFitScore, sheetAreaMM2, partArea))
                {
                    bestPack = std::move(packed);
                    bestFitScore = packedCapacity;
                    haveBest = true;
                }
            }
        }

        // Try moving a short tail of placements on each sheet, then reinsert
        // those parts plus any unplaced parts into the preserved free space.
        for (int pass = 0; pass < 2; ++pass)
        {
            const NestingPackResult repairSeed = bestPack;
            for (size_t sheetIndex = 0; sheetIndex < repairSeed.sheets.size(); ++sheetIndex)
            {
                const size_t maxTail = std::min<size_t>(
                    3, repairSeed.sheets[sheetIndex].placed.size());
                for (size_t tailCount = 1; tailCount <= maxTail; ++tailCount)
                {
                    for (auto strategy : kStrategies)
                    {
                        auto repaired = core.repairTail(repairSeed,
                            group.sheetWidth, group.sheetHeight,
                            sheetIndex, tailCount, strategy);
                        const size_t repairedCapacity =
                            core.maxAdditionalFit(repaired, group.rects);
                        if (betterSheetLayout(repaired, bestPack,
                            repairedCapacity, bestFitScore,
                            sheetAreaMM2, partArea))
                        {
                            bestPack = std::move(repaired);
                            bestFitScore = repairedCapacity;
                        }
                    }
                }
            }
        }

        group.layouts = bestPack.sheets;
        group.unplaced = bestPack.unplaced;
        group.nestingErrors = bestPack.validationErrors;
        if (!bestPack.isValid())
        {
            result.valid = false;
            result.errors.insert(result.errors.end(), group.nestingErrors.begin(), group.nestingErrors.end());
            for (const auto& item : group.unplaced)
                result.errors.push_back(item.reason + " (" + std::to_string(item.quantity) + " unplaced)");
        }

        result.sheets.insert(
            result.sheets.end(),
            group.layouts.begin(),
            group.layouts.end());

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
