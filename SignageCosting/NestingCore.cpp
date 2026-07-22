#include "NestingCore.h"
#include <algorithm>
#include <limits>
#include <iostream>

namespace
{
    constexpr bool DEBUG_TRY_PLACE = false;
}

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
        if constexpr (DEBUG_TRY_PLACE)
        {
            std::cout << "[TRY] FAIL - no free rects\n";
        }

        return false;
    }

    double bestScore = std::numeric_limits<double>::max();
    int bestIndex = -1;
    bool bestRotated = false;
    double bestX = 0;
    double bestY = 0;

    if constexpr (DEBUG_TRY_PLACE)
    {
        std::cout << "\n[TRY PLACE]\n";
        std::cout << "FreeRects: " << sheet.freeRects.size()
            << " | Item: " << w << "x" << h << "\n";
    }

    for (int i = 0; i < (int)sheet.freeRects.size(); i++)
    {
        const auto& fr = sheet.freeRects[i];

        if constexpr (DEBUG_TRY_PLACE)
        {
            std::cout << "  FR[" << i << "] "
                << "x=" << fr.x
                << " y=" << fr.y
                << " w=" << fr.width
                << " h=" << fr.height << "\n";
        }

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

    if constexpr (DEBUG_TRY_PLACE)
    {
        std::cout << "[TRY] SUCCESS -> index=" << bestIndex
            << " rotated=" << bestRotated
            << " x=" << bestX
            << " y=" << bestY
            << " score=" << bestScore << "\n";
    }

    outX = bestX;
    outY = bestY;
    rotated = bestRotated;
    usedIndex = bestIndex;

    return true;
}

void NestingCore::splitFreeRect(
    Sheet& sheet,
    int index,
    double x,
    double y,
    double w,
    double h)
{

    if (index < 0 || index >= (int)sheet.freeRects.size())
        return;

    PlacedRect free = sheet.freeRects[index];
    sheet.freeRects.erase(sheet.freeRects.begin() + index);

    // Left
    if (x > free.x)
    {
        PlacedRect r;
        r.x = free.x;
        r.y = free.y;
        r.width = x - free.x;
        r.height = free.height;

        if (r.width > 0 && r.height > 0)
            sheet.freeRects.push_back(r);
    }

    // Right
    if (x + w < free.x + free.width)
    {
        PlacedRect r;
        r.x = x + w;
        r.y = free.y;
        r.width = (free.x + free.width) - (x + w);
        r.height = free.height;

        if (r.width > 0 && r.height > 0)
            sheet.freeRects.push_back(r);
    }

    // Bottom
    if (y > free.y)
    {
        PlacedRect r;
        r.x = free.x;
        r.y = free.y;
        r.width = free.width;
        r.height = y - free.y;

        if (r.width > 0 && r.height > 0)
            sheet.freeRects.push_back(r);
    }

    // Top
    if (y + h < free.y + free.height)
    {
        PlacedRect r;
        r.x = free.x;
        r.y = y + h;
        r.width = free.width;
        r.height = (free.y + free.height) - (y + h);

        if (r.width > 0 && r.height > 0)
            sheet.freeRects.push_back(r);
    }

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

void NestingCore::pruneMaxRects(Sheet& sheet)
{
    for (int i = 0; i < (int)sheet.freeRects.size(); i++)
    {
        for (int j = i + 1; j < (int)sheet.freeRects.size(); j++)
        {
            const PlacedRect& a = sheet.freeRects[i];
            const PlacedRect& b = sheet.freeRects[j];

            // -----------------------------
            // A INSIDE B - remove A
            // -----------------------------
            if (a.x >= b.x &&
                a.y >= b.y &&
                a.x + a.width <= b.x + b.width &&
                a.y + a.height <= b.y + b.height)
            {
                sheet.freeRects.erase(sheet.freeRects.begin() + i);
                i--;
                goto next_i;
            }

            // -----------------------------
            // B INSIDE A - remove B
            // -----------------------------
            if (b.x >= a.x &&
                b.y >= a.y &&
                b.x + b.width <= a.x + a.width &&
                b.y + b.height <= a.y + a.height)
            {
                sheet.freeRects.erase(sheet.freeRects.begin() + j);
                j--;
            }
        }

    next_i:
        continue;
    }
}

std::vector<Sheet> NestingCore::pack(

    const std::vector<Rect>& items,
    double containerWidth,
    double containerHeight,
    PlacementStrategy strategy)
{

    std::cout << "\n===== NESTING CORE =====\n";
    std::cout << "Container Width : " << containerWidth << "\n";
    std::cout << "Container Height: " << containerHeight << "\n";

    // ---------------------------------------
    // Sort largest rectangles first
    // ---------------------------------------

    std::vector<Rect> sortedItems = items;

    std::sort(
        sortedItems.begin(),
        sortedItems.end(),
        [](const Rect& a, const Rect& b)
        {
            return (a.width * a.height) >
                (b.width * b.height);
        });

    std::vector<Sheet> sheets;

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
                continue;

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

                    if (index >= 0 && index < (int)sheet.freeRects.size())
                    {
                        double usedW = rotated ? item.height : item.width;
                        double usedH = rotated ? item.width : item.height;

                        splitFreeRect(sheet, index, x, y, usedW, usedH);
                    }

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
                    continue;
                }

                PlacedRect p;
                p.x = x;
                p.y = y;
                p.width = rotated ? item.height : item.width;
                p.height = rotated ? item.width : item.height;
                p.rotated = rotated;

                sheet.placed.push_back(p);

                if (index >= 0 && index < (int)sheet.freeRects.size())
                {
                    double usedW = rotated ? item.height : item.width;
                    double usedH = rotated ? item.width : item.height;

                    splitFreeRect(sheet, index, x, y, usedW, usedH);
                }
            }
        }
    }

    return sheets;
}