#include "MainMenuScreen.h"
#include "VerticalLayout.h"
#include "HorizontalLayout.h"
#include "Label.h"
#include "Metrics.h"
#include "PageHeader.h"
#include "NavigationItem.h"
#include <utility>

MainMenuScreen::MainMenuScreen()
{

    setLayout(std::make_unique<VerticalLayout>());

    auto headerPanel = std::make_shared<PageHeaderPanel>();
    headerPanel->setLayout(std::make_unique<HorizontalLayout>());
    configurePageHeader(headerPanel);

    addLayoutElement(
        headerPanel,
        SizePolicy::Fixed,
        Metrics::HeaderHeight);

    auto titlePanel = std::make_shared<PageHeaderContentPanel>();
    titlePanel->setLayout(std::make_unique<VerticalLayout>());
    configurePageHeaderTitle(titlePanel, Metrics::SpaceXS);
    addPageHeaderLogo(headerPanel);
    headerPanel->addLayoutElement(titlePanel, SizePolicy::Fill);

    auto workspaceLabel = std::make_shared<Label>();
    workspaceLabel->setText("WORKSPACE");
    workspaceLabel->setStyle(LabelStyle::Small);
    workspaceLabel->setTextTheme(TextTheme::Success);
    titlePanel->addLayoutElement(workspaceLabel, SizePolicy::Fixed, 16);

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

    //==================================================
    // Header
    //==================================================

    contentPanel = std::make_shared<Panel>();
    contentPanel->setLayout(std::make_unique<HorizontalLayout>());

    addLayoutElement(
        contentPanel,
        SizePolicy::Fill);

    sidebarPanel = std::make_shared<NavigationPanel>();
    sidebarPanel->setLayout(std::make_unique<VerticalLayout>());
    sidebarPanel->setSidebarStyle(true);
    sidebarPanel->setBorderVisible(false);
    sidebarPanel->setPadding(Metrics::SidebarPadding);
    sidebarPanel->setSpacing(Metrics::SpaceS);
    sidebarPanel->addBrand();

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

    contentPanel->addLayoutElement(
        sidebarPanel,
        SizePolicy::Fixed,
        Metrics::SidebarWidth);

    contentPanel->addLayoutElement(
        rightPanel,
        SizePolicy::Fill);

    sidebarPanel->addItem(
        "New Job",
        [this]()
        {
            if (newJobCallback)
                newJobCallback();
        });

    sidebarPanel->addItem(
        "Materials",
        {},
        "",
        false);

    sidebarPanel->addItem("Quotes", [this]()
    {
        if (quotesCallback)
            quotesCallback();
    });

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
