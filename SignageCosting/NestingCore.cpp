#include "NestingCore.h"
#include <algorithm>
#include <limits>
#include <cmath>
#include <utility>

void NestingCore::newSheet(
    std::vector<Sheet>& sheets,
    double width,
    double height)
{
    Sheet s;
    s.width = width;
    s.height = height;

    // REQUIRED: initialize full free space
    PlacedRect initial;
    initial.x = 0;
    initial.y = 0;
    initial.width = width;
    initial.height = height;
    initial.rotated = false;

    s.freeRects.clear();
    s.freeRects.push_back(initial);

    sheets.push_back(s);
}

bool NestingCore::rectanglesIntersect(
    const PlacedRect& a,
    const PlacedRect& b)
{
    if (a.x >= b.x + b.width)
        return false;

    if (a.x + a.width <= b.x)
        return false;

    if (a.y >= b.y + b.height)
        return false;

    if (a.y + a.height <= b.y)
        return false;

    return true;
}

bool NestingCore::tryPlaceInSheet(
    Sheet& sheet,
    double w,
    double h,
    PlacementStrategy strategy,
    bool& rotated,
    double& outX,
    double& outY,
    int& usedIndex)
{
    if (sheet.freeRects.empty())
    {
        return false;
    }

    double bestScore = std::numeric_limits<double>::max();
    int bestIndex = -1;
    bool bestRotated = false;
    double bestX = 0;
    double bestY = 0;

    for (int i = 0; i < (int)sheet.freeRects.size(); i++)
    {
        const auto& fr = sheet.freeRects[i];

        auto evaluate = [&](double rw, double rh, bool isRot)
            {
                double leftoverW = fr.width - rw;
                double leftoverH = fr.height - rh;

                if (leftoverW < 0 || leftoverH < 0)
                    return;

                double leftoverHoriz = fr.width - rw;
                double leftoverVert = fr.height - rh;

                double shortSideFit = std::min(leftoverHoriz, leftoverVert);
                double longSideFit = std::max(leftoverHoriz, leftoverVert);

                // tiny rotation penalty only
                double rotationPenalty = isRot ? 0.1 : 0.0;
                double areaFit =
                    (fr.width * fr.height) - (rw * rh);
                double score = 0.0;

                switch (strategy)
                {
                case PlacementStrategy::BestShortSideFit:
                    score = shortSideFit * 1000.0 +
                        longSideFit +
                        rotationPenalty;
                    break;

                case PlacementStrategy::BestLongSideFit:
                    score = longSideFit * 1000.0 +
                        shortSideFit +
                        rotationPenalty;
                    break;

                case PlacementStrategy::BestAreaFit:
                    score = areaFit +
                        rotationPenalty;
                    break;

                case PlacementStrategy::BottomLeft:
                    score =
                        fr.y * 1000000.0 +
                        fr.x;
                    break;

                case PlacementStrategy::BestContactPoint:
                {
                    constexpr double tolerance = 1e-7;
                    double contact = 0.0;
                    if (std::abs(fr.x) <= tolerance) contact += rh;
                    if (std::abs(fr.y) <= tolerance) contact += rw;
                    if (std::abs(fr.x + rw - sheet.width) <= tolerance) contact += rh;
                    if (std::abs(fr.y + rh - sheet.height) <= tolerance) contact += rw;

                    auto overlap = [](double a0, double a1, double b0, double b1)
                    {
                        return std::max(0.0, std::min(a1, b1) - std::max(a0, b0));
                    };
                    for (const auto& placed : sheet.placed)
                    {
                        if (std::abs(placed.x - (fr.x + rw)) <= tolerance ||
                            std::abs(placed.x + placed.width - fr.x) <= tolerance)
                            contact += overlap(fr.y, fr.y + rh,
                                placed.y, placed.y + placed.height);
                        if (std::abs(placed.y - (fr.y + rh)) <= tolerance ||
                            std::abs(placed.y + placed.height - fr.y) <= tolerance)
                            contact += overlap(fr.x, fr.x + rw,
                                placed.x, placed.x + placed.width);
                    }
                    score = -contact + rotationPenalty;
                    break;
                }
                }

                if (score < bestScore)
                {
                    bestScore = score;
                    bestIndex = i;
                    bestRotated = isRot;
                    bestX = fr.x;
                    bestY = fr.y;
                }
            };

        // NORMAL
        if (w <= fr.width && h <= fr.height)
            evaluate(w, h, false);

        // ROTATED
        if (h <= fr.width && w <= fr.height)
            evaluate(h, w, true);
    }

    if (bestIndex == -1)
    {
        return false;
    }

    outX = bestX;
    outY = bestY;
    rotated = bestRotated;
    usedIndex = bestIndex;

    return true;
}

void NestingCore::updateFreeRects(
    Sheet& sheet,
    double x,
    double y,
    double w,
    double h)
{

    std::vector<PlacedRect> remaining;
    remaining.reserve(sheet.freeRects.size() * 2);
    const PlacedRect used{ x, y, w, h, false };
    auto add = [&](double rx, double ry, double rw, double rh)
    {
        if (rw > 0.0 && rh > 0.0)
        {
            PlacedRect r;
            r.x = rx; r.y = ry; r.width = rw; r.height = rh;
            remaining.push_back(r);
        }
    };

    for (const auto& free : sheet.freeRects)
    {
        if (!rectanglesIntersect(free, used))
        {
            remaining.push_back(free);
            continue;
        }
        // MaxRects free regions overlap by design; split every intersecting one.
        add(free.x, free.y, x - free.x, free.height);
        add(x + w, free.y, free.x + free.width - (x + w), free.height);
        add(free.x, free.y, free.width, y - free.y);
        add(free.x, y + h, free.width, free.y + free.height - (y + h));
    }
    sheet.freeRects.swap(remaining);
    pruneFreeRects(sheet);
}

void NestingCore::pruneFreeRects(Sheet& sheet)
{
    for (size_t i = 0; i < sheet.freeRects.size(); ++i)
    {
        for (size_t j = i + 1; j < sheet.freeRects.size();)
        {
            const auto& a = sheet.freeRects[i];
            const auto& b = sheet.freeRects[j];

            // Is A completely inside B?
            if (a.x >= b.x &&
                a.y >= b.y &&
                a.x + a.width <= b.x + b.width &&
                a.y + a.height <= b.y + b.height)
            {
                sheet.freeRects.erase(sheet.freeRects.begin() + i);

                if (i > 0)
                    --i;

                break;
            }

            // Is B completely inside A?
            if (b.x >= a.x &&
                b.y >= a.y &&
                b.x + b.width <= a.x + a.width &&
                b.y + b.height <= a.y + a.height)
            {
                sheet.freeRects.erase(sheet.freeRects.begin() + j);
                continue;
            }

            ++j;
        }
    }
}

NestingPackResult NestingCore::pack(

    const std::vector<Rect>& items,
    double containerWidth,
    double containerHeight,
    PlacementStrategy strategy,
    ItemOrder order)
{

    NestingPackResult result;
    if (!std::isfinite(containerWidth) || !std::isfinite(containerHeight) ||
        containerWidth <= 0.0 || containerHeight <= 0.0)
    {
        result.validationErrors.push_back("Container dimensions must be finite and greater than zero.");
        return result;
    }

    // ---------------------------------------
    // Sort largest rectangles first
    // ---------------------------------------

    std::vector<Rect> sortedItems;
    sortedItems.reserve(items.size());
    for (const auto& item : items)
    {
        if (!std::isfinite(item.width) || !std::isfinite(item.height) ||
            item.width <= 0.0 || item.height <= 0.0 || item.quantity < 0)
        {
            result.validationErrors.push_back(
                "Item dimensions must be finite and greater than zero, and quantity cannot be negative.");
            if (item.quantity > 0)
                result.unplaced.push_back({ item.width, item.height, item.quantity,
                    "Invalid item dimensions or quantity." });
            continue;
        }
        if (item.quantity > 0)
            sortedItems.push_back(item);
    }

    auto orderKey = [order](const Rect& item)
    {
        const double longSide = std::max(item.width, item.height);
        const double shortSide = std::min(item.width, item.height);
        const double area = item.width * item.height;
        const double perimeter = item.width + item.height;
        switch (order)
        {
        case ItemOrder::LongSideDescending: return std::make_pair(longSide, area);
        case ItemOrder::ShortSideDescending: return std::make_pair(shortSide, area);
        case ItemOrder::WidthDescending: return std::make_pair(item.width, item.height);
        case ItemOrder::HeightDescending: return std::make_pair(item.height, item.width);
        case ItemOrder::PerimeterDescending: return std::make_pair(perimeter, area);
        case ItemOrder::AreaAscending: return std::make_pair(area, longSide);
        case ItemOrder::LongSideAscending: return std::make_pair(longSide, area);
        case ItemOrder::ShortSideAscending: return std::make_pair(shortSide, area);
        case ItemOrder::WidthAscending: return std::make_pair(item.width, item.height);
        case ItemOrder::HeightAscending: return std::make_pair(item.height, item.width);
        case ItemOrder::PerimeterAscending: return std::make_pair(perimeter, area);
        case ItemOrder::AreaDescending:
        default: return std::make_pair(area, longSide);
        }
    };
    const bool ascending =
        order == ItemOrder::AreaAscending ||
        order == ItemOrder::LongSideAscending ||
        order == ItemOrder::ShortSideAscending ||
        order == ItemOrder::WidthAscending ||
        order == ItemOrder::HeightAscending ||
        order == ItemOrder::PerimeterAscending;

    std::stable_sort(
        sortedItems.begin(),
        sortedItems.end(),
        [&](const Rect& a, const Rect& b)
        {
            return ascending ? orderKey(a) < orderKey(b) : orderKey(a) > orderKey(b);
        });

    if (sortedItems.empty())
        return result;

    std::vector<Sheet>& sheets = result.sheets;

    newSheet(sheets, containerWidth, containerHeight);

    for (const auto& item : sortedItems)
    {
        for (int q = 0; q < item.quantity; q++)
        {
            bool placed = false;

            // PRE-FIT CHECK
            bool canFit =
                (item.width <= containerWidth && item.height <= containerHeight) ||
                (item.height <= containerWidth && item.width <= containerHeight);

            if (!canFit)
            {
                result.unplaced.push_back({ item.width, item.height, item.quantity - q,
                    "Item does not fit inside the container in either orientation." });
                break;
            }

            for (auto& sheet : sheets)
            {
                bool rotated = false;
                double x = 0;
                double y = 0;
                int index = -1;

                if (tryPlaceInSheet(
                    sheet,
                    item.width,
                    item.height,
                    strategy,
                    rotated,
                    x,
                    y,
                    index))

                {
                    PlacedRect p;
                    p.x = x;
                    p.y = y;
                    p.width = rotated ? item.height : item.width;
                    p.height = rotated ? item.width : item.height;
                    p.rotated = rotated;

                    sheet.placed.push_back(p);

                    double usedW = rotated ? item.height : item.width;
                    double usedH = rotated ? item.width : item.height;
                    updateFreeRects(sheet, x, y, usedW, usedH);

                    placed = true;
                    break;
                }
            }

            // ----------------------------
            // FALLBACK SHEET CREATION
            // ----------------------------
            if (!placed)
            {
                newSheet(sheets, containerWidth, containerHeight);

                auto& sheet = sheets.back();

                bool rotated = false;
                double x = 0;
                double y = 0;
                int index = -1;

                if (!tryPlaceInSheet(sheet, item.width, item.height, strategy,
                    rotated, x, y, index))
                {
                    sheets.pop_back();
                    result.unplaced.push_back({ item.width, item.height, item.quantity - q,
                        "No valid placement was found in a new container." });
                    break;
                }

                PlacedRect p;
                p.x = x;
                p.y = y;
                p.width = rotated ? item.height : item.width;
                p.height = rotated ? item.width : item.height;
                p.rotated = rotated;

                sheet.placed.push_back(p);

                double usedW = rotated ? item.height : item.width;
                double usedH = rotated ? item.width : item.height;
                updateFreeRects(sheet, x, y, usedW, usedH);
            }
        }
    }

    auto layoutErrors = validateLayout(result.sheets, containerWidth, containerHeight);
    result.validationErrors.insert(result.validationErrors.end(),
        layoutErrors.begin(), layoutErrors.end());
    return result;
}

NestingPackResult NestingCore::repairTail(
    const NestingPackResult& seed,
    double containerWidth,
    double containerHeight,
    size_t sheetIndex,
    size_t tailCount,
    PlacementStrategy strategy)
{
    if (sheetIndex >= seed.sheets.size() || tailCount == 0 ||
        tailCount > seed.sheets[sheetIndex].placed.size())
        return seed;

    NestingPackResult repaired = seed;
    std::vector<Rect> pending;
    auto& target = repaired.sheets[sheetIndex];
    const size_t firstRemoved = target.placed.size() - tailCount;
    for (size_t i = firstRemoved; i < target.placed.size(); ++i)
    {
        const auto& placed = target.placed[i];
        pending.push_back({
            placed.rotated ? placed.height : placed.width,
            placed.rotated ? placed.width : placed.height,
            1
        });
    }
    target.placed.erase(target.placed.begin() + firstRemoved, target.placed.end());

    for (const auto& unplaced : seed.unplaced)
        pending.push_back({ unplaced.width, unplaced.height, unplaced.quantity });

    for (const auto& item : pending)
    {
        if (!std::isfinite(item.width) || !std::isfinite(item.height) ||
            item.width <= 0.0 || item.height <= 0.0 || item.quantity < 0)
            return seed;
    }

    repaired.unplaced.clear();
    repaired.validationErrors.clear();

    // Rebuild each sheet's free-space state around the placements being kept.
    for (auto& sheet : repaired.sheets)
    {
        sheet.freeRects.clear();
        PlacedRect full;
        full.width = containerWidth;
        full.height = containerHeight;
        sheet.freeRects.push_back(full);
        for (const auto& placed : sheet.placed)
            updateFreeRects(sheet, placed.x, placed.y, placed.width, placed.height);
    }

    std::stable_sort(pending.begin(), pending.end(),
        [](const Rect& a, const Rect& b)
        {
            return a.width * a.height > b.width * b.height;
        });

    for (const auto& item : pending)
    {
        for (int q = 0; q < item.quantity; ++q)
        {
            bool placed = false;
            for (auto& sheet : repaired.sheets)
            {
                bool rotated = false;
                double x = 0.0;
                double y = 0.0;
                int usedIndex = -1;
                if (!tryPlaceInSheet(sheet, item.width, item.height, strategy,
                    rotated, x, y, usedIndex))
                    continue;

                PlacedRect part;
                part.x = x;
                part.y = y;
                part.width = rotated ? item.height : item.width;
                part.height = rotated ? item.width : item.height;
                part.rotated = rotated;
                sheet.placed.push_back(part);
                updateFreeRects(sheet, x, y, part.width, part.height);
                placed = true;
                break;
            }

            if (!placed)
            {
                newSheet(repaired.sheets, containerWidth, containerHeight);
                auto& sheet = repaired.sheets.back();
                bool rotated = false;
                double x = 0.0;
                double y = 0.0;
                int usedIndex = -1;
                if (tryPlaceInSheet(sheet, item.width, item.height, strategy,
                    rotated, x, y, usedIndex))
                {
                    PlacedRect part;
                    part.x = x;
                    part.y = y;
                    part.width = rotated ? item.height : item.width;
                    part.height = rotated ? item.width : item.height;
                    part.rotated = rotated;
                    sheet.placed.push_back(part);
                    updateFreeRects(sheet, x, y, part.width, part.height);
                }
                else
                {
                    repaired.sheets.pop_back();
                    repaired.unplaced.push_back({ item.width, item.height,
                        item.quantity - q, "No valid placement was found during local repair." });
                    break;
                }
            }
        }
    }

    repaired.validationErrors = validateLayout(
        repaired.sheets, containerWidth, containerHeight);
    return repaired;
}

size_t NestingCore::maxAdditionalFit(
    const NestingPackResult& layout,
    const std::vector<Rect>& itemTypes)
{
    size_t bestCapacity = 0;
    std::vector<std::pair<double, double>> evaluatedTypes;
    for (const auto& item : itemTypes)
    {
        if (!std::isfinite(item.width) || !std::isfinite(item.height) ||
            item.width <= 0.0 || item.height <= 0.0)
            continue;
        const auto normalized = std::minmax(item.width, item.height);
        const std::pair<double, double> typeKey{ normalized.first, normalized.second };
        if (std::find(evaluatedTypes.begin(), evaluatedTypes.end(), typeKey) != evaluatedTypes.end())
            continue;
        evaluatedTypes.push_back(typeKey);

        std::vector<Sheet> trialSheets = layout.sheets;
        size_t fitCount = 0;
        while (fitCount < 32)
        {
            bool placed = false;
            for (auto& sheet : trialSheets)
            {
                bool rotated = false;
                double x = 0.0;
                double y = 0.0;
                int usedIndex = -1;
                if (!tryPlaceInSheet(sheet, item.width, item.height,
                    PlacementStrategy::BestShortSideFit,
                    rotated, x, y, usedIndex))
                    continue;

                PlacedRect part;
                part.x = x;
                part.y = y;
                part.width = rotated ? item.height : item.width;
                part.height = rotated ? item.width : item.height;
                part.rotated = rotated;
                sheet.placed.push_back(part);
                updateFreeRects(sheet, x, y, part.width, part.height);
                placed = true;
                ++fitCount;
                break;
            }
            if (!placed)
                break;
        }
        bestCapacity = std::max(bestCapacity, fitCount);
    }
    return bestCapacity;
}

std::vector<std::string> NestingCore::validateLayout(
    const std::vector<Sheet>& sheets,
    double containerWidth,
    double containerHeight)
{
    std::vector<std::string> errors;
    constexpr double tolerance = 1e-7;
    for (size_t s = 0; s < sheets.size(); ++s)
    {
        const auto& sheet = sheets[s];
        if (!std::isfinite(sheet.width) || !std::isfinite(sheet.height) ||
            sheet.width <= 0.0 || sheet.height <= 0.0 ||
            sheet.width > containerWidth + tolerance || sheet.height > containerHeight + tolerance)
            errors.push_back("Sheet " + std::to_string(s + 1) + " has invalid container bounds.");

        for (size_t i = 0; i < sheet.placed.size(); ++i)
        {
            const auto& p = sheet.placed[i];
            if (!std::isfinite(p.x) || !std::isfinite(p.y) ||
                !std::isfinite(p.width) || !std::isfinite(p.height) ||
                p.width <= 0.0 || p.height <= 0.0 || p.x < -tolerance || p.y < -tolerance ||
                p.x + p.width > sheet.width + tolerance || p.y + p.height > sheet.height + tolerance)
            {
                errors.push_back("Sheet " + std::to_string(s + 1) + " contains a part outside its bounds.");
                continue;
            }
            for (size_t j = i + 1; j < sheet.placed.size(); ++j)
                if (rectanglesIntersect(p, sheet.placed[j]))
                    errors.push_back("Sheet " + std::to_string(s + 1) + " contains overlapping parts.");
        }
    }
    return errors;
}
