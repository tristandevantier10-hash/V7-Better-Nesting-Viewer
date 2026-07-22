#pragma once

#include <memory>
#include "UIElement.h"

enum class SizePolicy
{
    Fixed,
    Fill
};

struct LayoutItem
{
    std::shared_ptr<UIElement> element;

    SizePolicy policy = SizePolicy::Fill;

    int size = 0;
};
