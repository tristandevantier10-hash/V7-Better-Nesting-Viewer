#pragma once

#include "NestingEngine.h"
#include <stdexcept>

namespace NestingGeometry
{
    inline std::vector<Part> rectangleParts(const std::vector<Rect>& items)
    {
        std::vector<Part> result;
        for (const auto& item : items)
            result.push_back({item.partId,rectangle(item.width,item.height),item.quantity,{0,90}});
        return result;
    }
    inline std::vector<SheetLayout> contourLayouts(const std::vector<Sheet>& sheets)
    {
        std::vector<SheetLayout> result;
        for (const auto& sheet : sheets)
        {
            SheetLayout layout{sheet.width,sheet.height,{}};
            for (const auto& part : sheet.placed)
                layout.placements.push_back({part.partId,part.instanceIndex,part.contour ? part.contourTransform :
                    Transform{part.x+(part.rotated ? part.width : 0),part.y,part.rotated ? 90.0 : 0.0}});
            result.push_back(std::move(layout));
        }
        return result;
    }
}
