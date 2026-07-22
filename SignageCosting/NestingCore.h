#pragma once

#include <vector>
#include "NestingEngine.h"
#include "PlacementStrategy.h"

class NestingCore
{
public:

    std::vector<Sheet> pack(
        const std::vector<Rect>& items,
        double containerWidth,
        double containerHeight,
        PlacementStrategy strategy
    );

private:

    void newSheet(
        std::vector<Sheet>& sheets,
        double width,
        double height
    );

    bool tryPlaceInSheet(
        Sheet& sheet,
        double w,
        double h,
        PlacementStrategy strategy,
        bool& rotated,
        double& outX,
        double& outY,
        int& usedIndex
    );

    void splitFreeRect(
        Sheet& sheet,
        int index,
        double x,
        double y,
        double w,
        double h
    );

    // NEW
    void pruneFreeRects(
        Sheet& sheet
    );

    void pruneMaxRects(Sheet& sheet);

    // SAFETY GUARD (does NOT break existing code)
    bool isValidIndex(const Sheet& sheet, int index);

    bool rectanglesIntersect(
        const PlacedRect& a,
        const PlacedRect& b);
};