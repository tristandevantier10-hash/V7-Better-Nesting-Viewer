#pragma once

#include "Metrics.h"
#include "NavigationItem.h"
#include "NavigationPanel.h"
#include "UIElement.h"
#include "VerticalLayout.h"
#include <functional>
#include <memory>

enum class PrimaryNavigationRoute
{
    Overview,
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
    std::function<void(PrimaryNavigationRoute)> onNavigate,
    bool newQuoteMode = false)
{
    auto panel = std::make_shared<NavigationPanel>();
    panel->setLayout(std::make_unique<VerticalLayout>());
    panel->setSidebarStyle(true);
    panel->setBorderVisible(false);
    panel->setPadding(Metrics::SidebarPadding);
    panel->setSpacing(Metrics::SpaceS);
    panel->addDashboardBrand();
    panel->addLayoutElement(
        std::make_shared<UIElement>(),
        SizePolicy::Fixed,
        Metrics::DashboardNavigationTopGap);

    if (newQuoteMode)
    {
        panel->addLayoutElement(
            std::make_shared<UIElement>(),
            SizePolicy::Fill);
        panel->addItem(
            "Cancel",
            [onNavigate]()
            {
                if (onNavigate)
                    onNavigate(PrimaryNavigationRoute::Overview);
            },
            "Assets/Icons/back.svg",
            true);
        return panel;
    }

    std::shared_ptr<NavigationItem> selectedItem;
    auto addItem = [&panel, &selectedItem, selectedRoute, &onNavigate](
        const char* label,
        PrimaryNavigationRoute route,
        const char* iconPath,
        bool enabled)
    {
        auto item = panel->addItem(
            label,
            [onNavigate, route]()
            {
                if (onNavigate)
                    onNavigate(route);
            },
            iconPath,
            enabled);
        if (route == selectedRoute)
            selectedItem = item;
    };

    panel->addHeading("GENERAL");
    addItem("Overview", PrimaryNavigationRoute::Overview, "Assets/Icons/overview.svg", true);

    panel->addHeading("QUOTES");
    addItem("New Quote", PrimaryNavigationRoute::NewJob, "Assets/Icons/new_job.svg", true);
    addItem("Quote History", PrimaryNavigationRoute::Quotes, "Assets/Icons/quotes.svg", true);

    panel->addHeading("MANAGE");
    addItem("Materials", PrimaryNavigationRoute::MaterialDatabase, "Assets/Icons/materials.svg", false);
    addItem("Customers", PrimaryNavigationRoute::CustomerDatabase, "Assets/Icons/customers.svg", true);
    addItem("Labour Rates", PrimaryNavigationRoute::LabourRates, "Assets/Icons/labour.svg", true);
    addItem("Settings", PrimaryNavigationRoute::Settings, "Assets/Icons/settings.svg", true);

    panel->addLayoutElement(
        std::make_shared<UIElement>(),
        SizePolicy::Fill);
    addItem("Exit", PrimaryNavigationRoute::Exit, "Assets/Icons/exit.svg", true);

    panel->selectItem(selectedItem);

    return panel;
}
