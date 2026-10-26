#include "MainMenuScreen.h"
#include "VerticalLayout.h"
#include "HorizontalLayout.h"
#include "Label.h"
#include "Metrics.h"
#include "PageHeader.h"
#include "NavigationItem.h"
#include "UIElement.h"
#include <utility>

MainMenuScreen::MainMenuScreen()
{
    setLayout(std::make_unique<HorizontalLayout>());
    setPadding(0);
    setSpacing(0);

    auto headerPanel = std::make_shared<DashboardBannerPanel>();
    headerPanel->setLayout(std::make_unique<HorizontalLayout>());
    configurePageHeader(headerPanel);
    headerPanel->setDividerVisible(false);
    headerPanel->setBottomAccentVisible(false);

    auto titlePanel = std::make_shared<PageHeaderContentPanel>();
    titlePanel->setLayout(std::make_unique<VerticalLayout>());
    configurePageHeaderTitle(titlePanel, Metrics::SpaceXS);
    titlePanel->setPadding(Metrics::SpaceL);
    headerPanel->addLayoutElement(titlePanel, SizePolicy::Fill);

    auto dashboardLabel = std::make_shared<Label>();
    dashboardLabel->setText("Dashboard");
    dashboardLabel->setStyle(LabelStyle::Heading);
    configurePageHeaderTitleLabel(dashboardLabel);

    titlePanel->addLayoutElement(
        dashboardLabel,
        SizePolicy::Fixed,
        30);

    auto subtitleLabel = std::make_shared<Label>();
    subtitleLabel->setText("Signage Costing Suite");
    subtitleLabel->setStyle(LabelStyle::Small);
    configurePageHeaderSubtitleLabel(subtitleLabel);

    titlePanel->addLayoutElement(
        subtitleLabel,
        SizePolicy::Fixed,
        18);

    headerPanel->addLayoutElement(
        std::make_shared<DashboardHeaderControls>(),
        SizePolicy::Fixed,
        236);

    sidebarPanel = std::make_shared<NavigationPanel>();
    sidebarPanel->setLayout(std::make_unique<VerticalLayout>());
    sidebarPanel->setSidebarStyle(true);
    sidebarPanel->setBorderVisible(false);
    sidebarPanel->setPadding(Metrics::SidebarPadding);
    sidebarPanel->setSpacing(Metrics::SpaceS);
    sidebarPanel->addDashboardBrand();

    contentPanel = std::make_shared<Panel>();
    contentPanel->setLayout(std::make_unique<VerticalLayout>());
    contentPanel->setPadding(0);
    contentPanel->setSpacing(0);

    auto rightPanel = std::make_shared<Panel>();
    rightPanel->setLayout(std::make_unique<VerticalLayout>());
    rightPanel->setPadding(Metrics::SpaceL);
    rightPanel->setSpacing(Metrics::SpaceL);

    dashboardContent = std::make_shared<DashboardContent>([this]()
    {
        if (quotesCallback)
            quotesCallback();
    });

    rightPanel->addLayoutElement(dashboardContent, SizePolicy::Fill);

    contentPanel->addLayoutElement(headerPanel, SizePolicy::Fixed, 132);
    contentPanel->addLayoutElement(rightPanel, SizePolicy::Fill);

    addLayoutElement(sidebarPanel, SizePolicy::Fixed, Metrics::SidebarWidth);
    addLayoutElement(contentPanel, SizePolicy::Fill);

    sidebarPanel->addHeading("GENERAL");
    overviewItem = sidebarPanel->addItem(
        "Overview",
        []() {},
        "Assets/Icons/overview.svg");

    sidebarPanel->addHeading("QUOTES");
    sidebarPanel->addItem(
        "New Quote",
        [this]()
        {
            if (newJobCallback)
                newJobCallback();
        },
        "Assets/Icons/new_job.svg");

    sidebarPanel->addItem("Quote History", [this]()
    {
        if (quotesCallback)
            quotesCallback();
    });

    sidebarPanel->addHeading("MANAGE");
    sidebarPanel->addItem(
        "Materials",
        {},
        "",
        false);

    sidebarPanel->addItem(
        "Customers",
        [this]()
        {
            if (customerDatabaseCallback)
                customerDatabaseCallback();
        });

    labourRatesItem = sidebarPanel->addItem("Labour Rates", [this]()
    {
        if (labourRatesCallback)
            labourRatesCallback();
    });

    sidebarPanel->addItem(
        "Settings",
        [this]()
        {
            if (settingsCallback)
                settingsCallback();
        });

    sidebarPanel->addLayoutElement(
        std::make_shared<UIElement>(),
        SizePolicy::Fill);

    sidebarPanel->addItem(
        "Exit",
        [this]()
        {
            if (exitCallback)
                exitCallback();
        });

}

void MainMenuScreen::update(const SDL_Event& e)
{
    Screen::update(e);
}

void MainMenuScreen::render(Renderer& renderer)
{
    if (sidebarPanel && overviewItem)
        sidebarPanel->selectItem(overviewItem);

    Screen::render(renderer);
}

void MainMenuScreen::setNewJobCallback(std::function<void()> callback)
{
    newJobCallback = callback;
}

void MainMenuScreen::setQuotesCallback(std::function<void()> callback)
{
    quotesCallback = callback;
}

void MainMenuScreen::setCustomerDatabaseCallback(
    std::function<void()> callback)
{
    customerDatabaseCallback = callback;
}

void MainMenuScreen::setExitCallback(std::function<void()> callback)
{
    exitCallback = callback;
}

void MainMenuScreen::setSettingsCallback(std::function<void()> callback)
{
    settingsCallback = callback;
}

void MainMenuScreen::setLabourRatesCallback(std::function<void()> callback)
{
    labourRatesCallback = std::move(callback);
}

void MainMenuScreen::setLabourRatesAvailable(bool available)
{
    if (labourRatesItem)
        labourRatesItem->setEnabled(available);
}

void MainMenuScreen::refreshStatus()
{
    if (dashboardContent)
        dashboardContent->refresh();
}
