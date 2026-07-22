#include "Layout.h"
#include "UIElement.h"

void Layout::addElement(
    std::shared_ptr<UIElement> element,
    SizePolicy policy,
    int size)
{
    LayoutItem item;

    item.element = element;
    item.policy = policy;
    item.size = size;

    items.push_back(item);
}

void Layout::clear()
{
    items.clear();
}