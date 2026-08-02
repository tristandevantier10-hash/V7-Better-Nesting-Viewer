#include "MainMenuScreen.h"
#include <iostream>
#include "Renderer.h"
#include "VerticalLayout.h"
#include "HorizontalLayout.h"
#include "TextBox.h"
#include "ComboBox.h"
#include "MaterialDatabase.h"
#include "NavigationItem.h"
#include "Metrics.h"

MainMenuScreen::MainMenuScreen()
{

    setLayout(std::make_unique<VerticalLayout>());

    auto headerPanel = std::make_shared<Panel>();
    headerPanel->setLayout(std::make_unique<HorizontalLayout>());
    headerPanel->setHeaderStyle(true);
    headerPanel->setBorderVisible(false);
    headerPanel->setPadding(Metrics::HeaderPadding);
    headerPanel->setSpacing(Metrics::SpaceM);

    addLayoutElement(
        headerPanel,
        SizePolicy::Fixed,
        Metrics::HeaderHeight);

    auto dashboardLabel = std::make_shared<Label>();
    dashboardLabel->setText("Dashboard");
    dashboardLabel->setStyle(LabelStyle::Heading);
    dashboardLabel->setTextColour(DefaultTheme.accent);

    headerPanel->addLayoutElement(
        dashboardLabel,
        SizePolicy::Fill);

    auto subtitleLabel = std::make_shared<Label>();
    subtitleLabel->setText("Signage Costing Suite");
    subtitleLabel->setStyle(LabelStyle::Small);

    headerPanel->addLayoutElement(
        subtitleLabel,
        SizePolicy::Fixed,
        Metrics::LargeRow);

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

    auto rightPanel = std::make_shared<Panel>();
    rightPanel->setLayout(std::make_unique<VerticalLayout>());
    rightPanel->setPadding(20);
    rightPanel->setSpacing(20);

    statusPanel = std::make_shared<CardPanel>();
    statusPanel->setTitle("System Status");
    statusPanel->setBorderVisible(false);
    statusPanel->setPadding(Metrics::PanelPadding);
    statusPanel->setSpacing(Metrics::SpaceS);

    rightPanel->addLayoutElement(
        statusPanel,
        SizePolicy::Fixed,
        260);

    contentPanel->addLayoutElement(
        sidebarPanel,
        SizePolicy::Fixed,
        Metrics::SidebarWidth);

    contentPanel->addLayoutElement(
        rightPanel,
        SizePolicy::Fill);

    materialsStatus = std::make_shared<Label>();
    materialsStatus->setText("[OK] Materials Loaded");
    materialsStatus->setStyle(LabelStyle::Small);

    statusPanel->addLayoutElement(
        materialsStatus,
        SizePolicy::Fixed,
        Metrics::NormalRow);

    pricingStatus = std::make_shared<Label>();
    pricingStatus->setText("[OK] Pricing Loaded");
    pricingStatus->setStyle(LabelStyle::Small);

    statusPanel->addLayoutElement(
        pricingStatus,
        SizePolicy::Fixed,
        Metrics::NormalRow);

    productionStatus = std::make_shared<Label>();
    productionStatus->setText("[OK] Production Pricing Loaded");
    productionStatus->setStyle(LabelStyle::Small);

    statusPanel->addLayoutElement(
        productionStatus,
        SizePolicy::Fixed,
        Metrics::NormalRow);

    materialCountLabel = std::make_shared<Label>();
    materialCountLabel->setText("Materials : 0");
    materialCountLabel->setStyle(LabelStyle::Small);    

    statusPanel->addLayoutElement(
        materialCountLabel,
        SizePolicy::Fixed,
        Metrics::NormalRow);

    variantCountLabel = std::make_shared<Label>();
    variantCountLabel->setText("Variants : 0");
    variantCountLabel->setStyle(LabelStyle::Small); 

    statusPanel->addLayoutElement(
        variantCountLabel,
        SizePolicy::Fixed,
        Metrics::NormalRow);

    auto newJobItem =
        sidebarPanel->addItem(
            "NEW JOB",
            [this]()
            {
                if (newJobCallback)
                    newJobCallback();
            });

    auto settingsItem =
        sidebarPanel->addItem(
            "SETTINGS",
            []()
            {
                std::cout << "Settings clicked\n";
            });

    auto exitItem =
        sidebarPanel->addItem(
            "EXIT",
            []()
            {
                std::cout << "Exit clicked\n";
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

void MainMenuScreen::refreshStatus()
{
    auto materials =
        MaterialDatabase::getBaseMaterials();

    materialCountLabel->setText(
        "Materials : " +
        std::to_string(materials.size()));

    int variantCount = 0;

    for (const auto& id : materials)
    {
        variantCount +=
            MaterialDatabase::get(id).variants.size();
    }

    variantCountLabel->setText(
        "Variants : " +
        std::to_string(variantCount));
}