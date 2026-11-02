#pragma once

#include "NestingCore.h"
#include <array>
#include <cmath>
#include <tuple>
#include <set>
#include <map>

// Shared sheet-layout search for the nesting workspace and quote material groups.
namespace NestingLayoutSearch
{
    inline constexpr std::array<PlacementStrategy, 5> strategies{
        PlacementStrategy::BestShortSideFit, PlacementStrategy::BestLongSideFit,
        PlacementStrategy::BestAreaFit, PlacementStrategy::BottomLeft,
        PlacementStrategy::BestContactPoint
    };
    inline constexpr std::array<ItemOrder, 12> orders{
        ItemOrder::AreaDescending, ItemOrder::LongSideDescending,
        ItemOrder::ShortSideDescending, ItemOrder::WidthDescending,
        ItemOrder::HeightDescending, ItemOrder::PerimeterDescending,
        ItemOrder::AreaAscending, ItemOrder::LongSideAscending,
        ItemOrder::ShortSideAscending, ItemOrder::WidthAscending,
        ItemOrder::HeightAscending, ItemOrder::PerimeterAscending
    };

    inline double envelopeArea(const Sheet& sheet)
    {
        double right = 0.0, bottom = 0.0;
        for (const auto& part : sheet.placed)
        {
            right = std::max(right, part.x + part.width);
            bottom = std::max(bottom, part.y + part.height);
        }
        return right * bottom;
    }

    inline double largestOffcut(const Sheet& sheet)
    {
        double area = 0.0;
        for (const auto& free : sheet.freeRects)
            area = std::max(area, free.width * free.height);
        return area;
    }

    inline size_t alignmentPenalty(const Sheet& sheet)
    {
        // Prefer consistent rows/columns for parts with the same orientation.
        std::set<std::tuple<double, double, double>> columns, rows;
        for (const auto& part : sheet.placed)
        {
            columns.emplace(part.width, part.height, part.x);
            rows.emplace(part.width, part.height, part.y);
        }
        return columns.size() + rows.size();
    }

    inline auto score(const NestingPackResult& result, const std::vector<Rect>& items = {})
    {
        double totalEnvelope = 0.0, reusable = 0.0;
        for (const auto& sheet : result.sheets)
        {
            totalEnvelope += envelopeArea(sheet);
            reusable += largestOffcut(sheet);
        }
        // Preserve the minimum sheet count, then keep the final sheet compact.
        // For equally compact layouts, prefer space that actually fits another
        // requested size before comparing raw offcut area (which can be thin strips).
        NestingCore core;
        const auto extraFit = core.maxAdditionalFit(result, items);
        return std::make_tuple(result.sheets.size(), envelopeArea(result.sheets.back()),
            -static_cast<int>(extraFit), alignmentPenalty(result.sheets.back()),
            -largestOffcut(result.sheets.back()), totalEnvelope, -reusable);
    }

    inline bool containsAll(const NestingPackResult& result, const std::vector<Rect>& items)
    {
        if (!result.isValid() || result.sheets.empty()) return false;
        // Compare quantities by original dimensions, allowing rotation. Area alone
        // would not detect a missing part replaced with another of equal area.
        std::vector<Rect> remaining = items;
        for (const auto& sheet : result.sheets)
        {
            for (const auto& part : sheet.placed)
            {
                bool matched = false;
                for (auto& item : remaining)
                {
                    const bool same = std::abs(part.width - item.width) < 1e-7 &&
                        std::abs(part.height - item.height) < 1e-7;
                    const bool rotated = std::abs(part.width - item.height) < 1e-7 &&
                        std::abs(part.height - item.width) < 1e-7;
                    if (item.quantity > 0 && (same || rotated) &&
                        (item.partId == 0 || item.partId == part.partId))
                    {
                        --item.quantity;
                        matched = true;
                        break;
                    }
                }
                if (!matched) return false;
            }
        }
        for (const auto& item : remaining) if (item.quantity != 0) return false;
        return true;
    }

    inline bool matches(const PlacedRect& part, const Rect& item)
    {
        return (item.partId == 0 || item.partId == part.partId) &&
            ((part.width == item.width && part.height == item.height) ||
            (part.height == item.width && part.width == item.height));
    }

    enum class SingleSheetGoal
    {
        PreserveCapacity,
        CleanFinishedLayout
    };

    inline auto finishedSheetScore(const NestingPackResult& result, const std::vector<Rect>& items)
    {
        const auto& sheet = result.sheets.front();
        NestingCore core;
        const auto capacity = core.maxAdditionalFit(result, items);
        // Demand is already placed: keep a usable spare slot, then favour
        // regular rows/columns over speculative capacity for many extra parts.
        return std::make_tuple(envelopeArea(sheet), capacity == 0,
            alignmentPenalty(sheet), -largestOffcut(sheet), -static_cast<int>(capacity));
    }

    inline NestingPackResult alignedRows(const std::vector<Rect>& items,
        double width, double height, bool rotate)
    {
        std::vector<Rect> sorted = items;
        std::stable_sort(sorted.begin(), sorted.end(), [](const Rect& a, const Rect& b)
        { return a.width * a.height > b.width * b.height; });
        Sheet sheet;
        sheet.width = width;
        sheet.height = height;
        double x = 0, y = 0, rowHeight = 0;
        for (const auto& item : sorted)
        {
            double w = rotate ? item.height : item.width;
            double h = rotate ? item.width : item.height;
            if (w > width && h <= width) std::swap(w, h);
            for (int quantity = 0; quantity < item.quantity; ++quantity)
            {
                // Start a new row when heights differ, rather than staggering
                // equal-sized parts around a shorter neighbour's edge.
                if (x > 0 && (x + w > width || h != rowHeight))
                {
                    y += rowHeight;
                    x = 0;
                    rowHeight = 0;
                }
                if (x + w > width || y + h > height) return {};
                sheet.placed.push_back({x, y, w, h, w != item.width || h != item.height, item.partId,
                    static_cast<std::uint32_t>(quantity)});
                x += w;
                rowHeight = h;
            }
        }
        NestingCore core;
        core.rebuildFreeSpace(sheet);
        NestingPackResult result;
        result.sheets.push_back(std::move(sheet));
        result.validationErrors = NestingCore::validateLayout(result.sheets, width, height);
        return result;
    }

    inline NestingPackResult packOne(const std::vector<Rect>& items, double width, double height,
        SingleSheetGoal goal = SingleSheetGoal::PreserveCapacity)
    {
        NestingCore core;
        NestingPackResult best;
        for (auto strategy : strategies)
            for (auto order : orders)
            {
                auto candidate = core.pack(items, width, height, strategy, order);
                if (candidate.sheets.size() != 1 || !containsAll(candidate, items)) continue;
                if (best.sheets.empty() || (goal == SingleSheetGoal::CleanFinishedLayout
                    ? finishedSheetScore(candidate, items) < finishedSheetScore(best, items)
                    : score(candidate, items) < score(best, items)))
                    best = std::move(candidate);
            }
        if (goal == SingleSheetGoal::CleanFinishedLayout)
            for (bool rotate : {false, true})
            {
                auto candidate = alignedRows(items, width, height, rotate);
                if (candidate.sheets.size() != 1 || !containsAll(candidate, items)) continue;
                if (best.sheets.empty() || finishedSheetScore(candidate, items) < finishedSheetScore(best, items))
                    best = std::move(candidate);
            }
        return best;
    }

    inline void consolidateSheets(NestingPackResult& result,
        const std::vector<Rect>& items, double width, double height)
    {
        NestingCore core;
        std::vector<Rect> types;
        for (const auto& item : items)
        {
            const auto found = std::find_if(types.begin(), types.end(), [&](const Rect& type)
            { return type.partId == item.partId &&
                ((type.width == item.width && type.height == item.height) ||
                (type.width == item.height && type.height == item.width)); });
            if (found == types.end() && item.quantity > 0) types.push_back(item);
        }
        for (size_t target = 0; target + 1 < result.sheets.size(); ++target)
        {
            for (const auto& type : types)
            {
                int available = 0;
                for (size_t donor = target + 1; donor < result.sheets.size(); ++donor)
                    for (const auto& part : result.sheets[donor].placed)
                        if (matches(part, type)) ++available;
                if (available == 0) continue;
                double used = 0;
                std::vector<Rect> base;
                for (const auto& part : result.sheets[target].placed)
                {
                    used += part.width * part.height;
                    base.push_back({part.rotated ? part.height : part.width,
                        part.rotated ? part.width : part.height, 1, part.partId});
                }
                const int upper = std::min(available, static_cast<int>(std::floor(
                    (width * height - used + 1e-7) / (type.width * type.height))));
                if (upper <= 0) continue;
                // Try actual repacks, rather than testing only the existing free
                // rectangles: a rotated corner part may need to move first.
                int low = 1, high = upper, transferred = 0;
                NestingPackResult replacement;
                while (low <= high)
                {
                    const int count = low + (high - low) / 2;
                    auto requested = base;
                    requested.push_back({type.width, type.height, count, type.partId});
                    auto trial = packOne(requested, width, height);
                    if (trial.sheets.size() == 1)
                    {
                        transferred = count;
                        replacement = std::move(trial);
                        low = count + 1;
                    }
                    else high = count - 1;
                }
                if (transferred == 0) continue;
                result.sheets[target] = std::move(replacement.sheets.front());
                for (size_t donor = target + 1; donor < result.sheets.size() && transferred > 0; ++donor)
                {
                    auto& sheet = result.sheets[donor];
                    for (auto part = sheet.placed.begin(); part != sheet.placed.end() && transferred > 0;)
                    {
                        if (matches(*part, type))
                        {
                            part = sheet.placed.erase(part);
                            --transferred;
                        }
                        else ++part;
                    }
                    core.rebuildFreeSpace(sheet);
                }
            }
        }
        result.sheets.erase(std::remove_if(result.sheets.begin(), result.sheets.end(),
            [](const Sheet& sheet) { return sheet.placed.empty(); }), result.sheets.end());
    }

    inline NestingPackResult findBest(const std::vector<Rect>& requested, double width, double height)
    {
        std::vector<Rect> items = requested;
        std::set<NestingGeometry::PartId> identities;
        for (const auto& item : items)
            if (item.partId != 0 && !identities.insert(item.partId).second)
            {
                NestingPackResult invalid;
                invalid.validationErrors.push_back("Source part identities must be unique.");
                return invalid;
            }
        // Legacy callers can omit IDs; derive repeatable IDs before sorting.
        NestingGeometry::PartId next = 1;
        for (auto& item : items)
            if (item.partId == 0)
            {
                while (identities.count(next)) ++next;
                item.partId = next;
                identities.insert(next++);
            }
        NestingCore core;
        NestingPackResult best;
        NestingPackResult partial;
        bool haveBest = false;
        bool havePartial = false;
        for (auto strategy : strategies)
        {
            for (auto order : orders)
            {
                auto candidate = core.pack(items, width, height, strategy, order);
                if (!containsAll(candidate, items))
                {
                    size_t count = 0, previousCount = 0;
                    for (const auto& sheet : candidate.sheets) count += sheet.placed.size();
                    for (const auto& sheet : partial.sheets) previousCount += sheet.placed.size();
                    if (!havePartial || count > previousCount ||
                        (count == previousCount && candidate.validationErrors.size() < partial.validationErrors.size()))
                    {
                        partial = std::move(candidate);
                        havePartial = true;
                    }
                    continue;
                }
                if (!haveBest || score(candidate, items) < score(best, items))
                {
                    best = std::move(candidate);
                    haveBest = true;
                }
            }
        }
        if (!haveBest)
        {
            // Preserve unplaced parts and errors so quote costing cannot treat
            // an unsuccessful search as a valid, zero-sheet result.
            if (partial.isValid())
                partial.validationErrors.push_back("No complete sheet layout was found.");
            return partial;
        }

        // Retain the quote engine's local repair search, using the same ranking
        // as the main search so it cannot undo final-sheet compactness.
        for (int pass = 0; pass < 2; ++pass)
        {
            const NestingPackResult seed = best;
            for (size_t sheetIndex = 0; sheetIndex < seed.sheets.size(); ++sheetIndex)
            {
                const size_t maxTail = std::min<size_t>(3, seed.sheets[sheetIndex].placed.size());
                for (size_t tail = 1; tail <= maxTail; ++tail)
                {
                    for (auto strategy : strategies)
                    {
                        auto repaired = core.repairTail(seed, width, height, sheetIndex, tail, strategy);
                        if (containsAll(repaired, items) && score(repaired, items) < score(best, items))
                            best = std::move(repaired);
                    }
                }
            }
        }

        // Fill earlier sheets with real parts from later sheets before tidying
        // every sheet. Never create an additional, unrequested part.
        const NestingPackResult beforeConsolidation = best;
        consolidateSheets(best, items, width, height);
        best.validationErrors = NestingCore::validateLayout(best.sheets, width, height);
        if (!containsAll(best, items)) best = beforeConsolidation;

        // Moving parts leaves holes on donor sheets. Repack every sheet after
        // consolidation, preserving its assigned sizes and quantities.
        for (auto& sheet : best.sheets)
        {
            std::vector<Rect> sheetItems;
            for (const auto& part : sheet.placed)
            {
                const double originalWidth = part.rotated ? part.height : part.width;
                const double originalHeight = part.rotated ? part.width : part.height;
                auto found = std::find_if(sheetItems.begin(), sheetItems.end(), [&](const Rect& item)
                { return item.partId == part.partId && item.width == originalWidth && item.height == originalHeight; });
                if (found == sheetItems.end()) sheetItems.push_back({originalWidth, originalHeight, 1, part.partId});
                else ++found->quantity;
            }
            auto candidate = packOne(sheetItems, width, height, SingleSheetGoal::CleanFinishedLayout);
            NestingPackResult current;
            current.sheets.push_back(sheet);
            if (candidate.sheets.size() == 1 &&
                finishedSheetScore(candidate, items) < finishedSheetScore(current, items))
                sheet = std::move(candidate.sheets.front());
        }
        // Independently revalidate the assembled layout before returning it.
        best.validationErrors = NestingCore::validateLayout(best.sheets, width, height);
        std::map<NestingGeometry::PartId, std::uint32_t> instances;
        for (auto& sheet : best.sheets)
            for (auto& part : sheet.placed) part.instanceIndex = instances[part.partId]++;
        if (!containsAll(best, items))
            best.validationErrors.push_back("Part identities or quantities changed during nesting.");
        return best;
    }
}
