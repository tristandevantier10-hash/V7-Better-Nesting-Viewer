#pragma once
#include "NestingGeometry.h"
#include "NestingCore.h"

namespace NestingService
{
    struct Options
    {
        double sheetWidth = 2440, sheetHeight = 1220;
        double clearance = 0, margin = 0;
    };
    struct Result
    {
        NestingPackResult layout;
        NestingGeometry::Method method = NestingGeometry::Method::Rectangles;
    };
    // Chooses the rectangle or contour calculation from actual geometry and
    // rotation/spacing constraints. Both return the same sheet/part structure.
    Result calculate(const std::vector<NestingGeometry::Part>& parts, const Options& options);
}
