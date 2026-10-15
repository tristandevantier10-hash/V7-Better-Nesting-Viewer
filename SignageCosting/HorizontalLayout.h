#pragma once

#include "Layout.h"

class HorizontalLayout : public Layout
{
public:

    void setCenterChildrenVertically(bool value)
    {
        centerChildrenVertically = value;
    }

    void performLayout(
        int x,
        int y,
        int w,
        int h,
        int padding) override;

private:
    bool centerChildrenVertically = false;
};
