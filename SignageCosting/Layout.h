#pragma once

#include <vector>
#include <memory>
#include "LayoutItem.h"

class Panel;

class Layout
{
public:

    virtual ~Layout() = default;

    void addPanel(
        std::shared_ptr<UIElement> element,
        SizePolicy policy = SizePolicy::Fill,
        int size = 0);

    void addElement(
        std::shared_ptr<UIElement> element,
        SizePolicy policy = SizePolicy::Fill,
        int size = 0);

    virtual void performLayout(
        int x,
        int y,
        int w,
        int h) = 0;

    void clear();

protected:

    std::vector<LayoutItem> items;
};
