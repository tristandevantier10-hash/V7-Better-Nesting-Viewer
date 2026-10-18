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
    LabourRates,
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
    panel->addBrand();

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

    addItem(
        selectedRoute == PrimaryNavigationRoute::NewJob ? "Job Input" : "New Job",
        PrimaryNavigationRoute::NewJob);
    addItem("Materials", PrimaryNavigationRoute::MaterialDatabase, false);
    addItem("Quotes", PrimaryNavigationRoute::Quotes);
    addItem("Customers", PrimaryNavigationRoute::CustomerDatabase);
    addItem("Labour Rates", PrimaryNavigationRoute::LabourRates);
    addItem("Settings", PrimaryNavigationRoute::Settings);
    addItem("Exit", PrimaryNavigationRoute::Exit);

    for (const auto& item : items)
        item.second->setSelected(item.first == selectedRoute);

    return panel;
}
