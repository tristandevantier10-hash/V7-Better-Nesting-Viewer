#pragma once

#include "Metrics.h"
#include "NavigationItem.h"
#include "NavigationPanel.h"
#include "VerticalLayout.h"
#include <functional>
#include <memory>
#include <utility>
#include <vector>

enum class PrimaryNavigationRoute
{
    NewJob,
    MaterialDatabase,
    Quotes,
    CustomerDatabase,
    Settings,
    Exit
};

inline std::shared_ptr<NavigationPanel> createPrimaryNavigationPanel(
    PrimaryNavigationRoute selectedRoute,
    std::function<void(PrimaryNavigationRoute)> onNavigate)
{
    auto panel = std::make_shared<NavigationPanel>();
    panel->setLayout(std::make_unique<VerticalLayout>());
    panel->setSidebarStyle(true);
    panel->setBorderVisible(false);
    panel->setPadding(Metrics::SidebarPadding);
    panel->setSpacing(Metrics::SpaceS);

    std::vector<std::pair<PrimaryNavigationRoute, std::shared_ptr<NavigationItem>>> items;
    auto addItem = [&panel, &items, &onNavigate](
        const char* label,
        PrimaryNavigationRoute route,
        bool enabled = true)
    {
        auto item = panel->addItem(
            label,
            [onNavigate, route]()
            {
                if (onNavigate)
                    onNavigate(route);
            },
            "",
            enabled);
        items.emplace_back(route, item);
    };

    addItem("NEW JOB", PrimaryNavigationRoute::NewJob);
    addItem("MATERIAL DATABASE", PrimaryNavigationRoute::MaterialDatabase, false);
    addItem("QUOTES", PrimaryNavigationRoute::Quotes);
    addItem("CUSTOMER DATABASE", PrimaryNavigationRoute::CustomerDatabase);
    addItem("SETTINGS", PrimaryNavigationRoute::Settings);
    addItem("EXIT", PrimaryNavigationRoute::Exit);

    for (const auto& item : items)
        item.second->setSelected(item.first == selectedRoute);

    return panel;
}
