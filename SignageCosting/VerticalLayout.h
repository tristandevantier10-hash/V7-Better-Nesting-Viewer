#pragma once

#include "Layout.h"

class VerticalLayout : public Layout
{
public:

    void performLayout(
        int x,
        int y,
        int w,
        int h,
        int padding) override;

    void setCenterContent(bool value)
    {
        centerContent = value;
    }

private:
    bool centerContent = false;
};
