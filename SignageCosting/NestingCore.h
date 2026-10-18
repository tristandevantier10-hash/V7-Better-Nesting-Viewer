#pragma once

#include <vector>
#include <string>
#include "NestingEngine.h"
#include "PlacementStrategy.h"

struct UnplacedRect
{
    double width = 0.0;
    double height = 0.0;
    int quantity = 0;
    std::string reason;
};

struct NestingPackResult
{
    std::vector<Sheet> sheets;
    std::vector<UnplacedRect> unplaced;
    std::vector<std::string> validationErrors;

    bool isValid() const
    {
        return unplaced.empty() && validationErrors.empty();
    }
};

enum class ItemOrder
{
    AreaDescending,
    LongSideDescending,
    ShortSideDescending,
    WidthDescending,
    HeightDescending,
    PerimeterDescending,
    AreaAscending,
    LongSideAscending,
    ShortSideAscending,
    WidthAscending,
    HeightAscending,
    PerimeterAscending
};

class NestingCore
{
public:

    NestingPackResult pack(
        const std::vector<Rect>& items,
        double containerWidth,
        double containerHeight,
        PlacementStrategy strategy,
        ItemOrder order = ItemOrder::AreaDescending
    );

    NestingPackResult repairTail(
        const NestingPackResult& seed,
        double containerWidth,
        double containerHeight,
        size_t sheetIndex,
        size_t tailCount,
        PlacementStrategy strategy);

    size_t maxAdditionalFit(
        const NestingPackResult& layout,
        const std::vector<Rect>& itemTypes);

    static std::vector<std::string> validateLayout(
        const std::vector<Sheet>& sheets,
        double containerWidth,
        double containerHeight);

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

    void updateFreeRects(
        Sheet& sheet,
        double x,
        double y,
        double w,
        double h
    );

    // NEW
    void pruneFreeRects(
        Sheet& sheet
    );

    static bool rectanglesIntersect(
        const PlacedRect& a,
        const PlacedRect& b);
};
