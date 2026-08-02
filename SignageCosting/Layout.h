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
        int h,
        int padding) = 0;

    void clear();

    void setSpacing(int value);

    int getSpacing() const;

    void setTopInset(int value);

    int getTopInset() const;

protected:

    std::vector<LayoutItem> items;

    int spacing = 0;

    int topInset = 0;
};
