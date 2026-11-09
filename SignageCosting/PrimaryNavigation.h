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
    Nesting,
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
    panel->setSpacing(Metrics::SpaceXS);
    panel->showWorkspaceEdge();

    if (newQuoteMode)
    {
        panel->addLayoutElement(
            std::make_shared<UIElement>(),
            SizePolicy::Fill);
        const auto cancelItem = panel->addItem(
            "Cancel",
            [onNavigate]()
            {
                if (onNavigate)
                    onNavigate(PrimaryNavigationRoute::Overview);
            },
            "Assets/Icons/Sidebar/back.svg",
            true);
        panel->selectItem(cancelItem);
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
    panel->addLayoutElement(std::make_shared<UIElement>(), SizePolicy::Fixed, Metrics::SpaceXS);
    addItem("Overview", PrimaryNavigationRoute::Overview, "Assets/Icons/Sidebar/overview.svg", true);
    addItem("Nesting", PrimaryNavigationRoute::Nesting, "Assets/Icons/Sidebar/nesting.svg", true);

    panel->addLayoutElement(std::make_shared<UIElement>(), SizePolicy::Fixed, 8);
    panel->addHeading("QUOTES");
    panel->addLayoutElement(std::make_shared<UIElement>(), SizePolicy::Fixed, Metrics::SpaceXS);
    addItem("New Quotes", PrimaryNavigationRoute::NewJob, "Assets/Icons/Sidebar/new_job.svg", true);
    addItem("Quote History", PrimaryNavigationRoute::Quotes, "Assets/Icons/Sidebar/quotes.svg", true);

    panel->addLayoutElement(std::make_shared<UIElement>(), SizePolicy::Fixed, 8);
    panel->addHeading("MANAGE");
    panel->addLayoutElement(std::make_shared<UIElement>(), SizePolicy::Fixed, Metrics::SpaceXS);
    addItem("Materials", PrimaryNavigationRoute::MaterialDatabase, "Assets/Icons/Sidebar/materials.svg", true);
    addItem("Customers", PrimaryNavigationRoute::CustomerDatabase, "Assets/Icons/Sidebar/customers.svg", true);
    addItem("Labour Rates", PrimaryNavigationRoute::LabourRates, "Assets/Icons/Sidebar/labour.svg", true);
    addItem("Settings", PrimaryNavigationRoute::Settings, "Assets/Icons/Sidebar/settings.svg", true);

    panel->addLayoutElement(
        std::make_shared<UIElement>(),
        SizePolicy::Fill);
    panel->addHeading("EXIT");
    panel->addLayoutElement(std::make_shared<UIElement>(), SizePolicy::Fixed, Metrics::SpaceXS);
    addItem("Close Esti-Mate", PrimaryNavigationRoute::Exit, "Assets/Icons/Sidebar/exit.svg", true);

    panel->selectItem(selectedItem);

    return panel;
}
