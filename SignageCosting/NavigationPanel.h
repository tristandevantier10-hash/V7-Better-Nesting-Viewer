#pragma once

#include "Panel.h"
#include <memory>
#include <vector>
#include <functional>
#include <string>

class NavigationItem;

class NavigationPanel : public Panel
{
public:

    NavigationPanel();

    std::shared_ptr<NavigationItem> addItem(
        const std::string& text,
        std::function<void()> callback);

private:

    std::vector<std::shared_ptr<NavigationItem>> items;
};