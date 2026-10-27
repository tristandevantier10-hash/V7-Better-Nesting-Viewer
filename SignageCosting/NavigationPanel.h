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
        std::function<void()> callback,
        const std::string& iconPath = "",
        bool enabled = true);

    void selectItem(const std::shared_ptr<NavigationItem>& item);
    void selectItemByText(const std::string& text);
    std::shared_ptr<NavigationItem> findItemByText(const std::string& text) const;

    void addHeading(
        const std::string& text);

    void addBrand();
    void addDashboardBrand();

private:

    std::vector<std::shared_ptr<NavigationItem>> items;
    bool hasBrand = false;
};
