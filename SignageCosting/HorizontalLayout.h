#pragma once

#include "Layout.h"

class HorizontalLayout : public Layout
{
public:

    void performLayout(
        int x,
        int y,
        int w,
        int h) override;
};
