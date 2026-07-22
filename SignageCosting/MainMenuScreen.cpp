#include "MainMenuScreen.h"
#include <iostream>
#include "Renderer.h"
#include "VerticalLayout.h"
#include "HorizontalLayout.h"
#include "TextBox.h"
#include "ComboBox.h"
#include "MaterialDatabase.h"

MainMenuScreen::MainMenuScreen()
{

    setLayout(std::make_unique<VerticalLayout>());

    //==================================================
    // Header
    //==================================================

    companyLabel = std::make_shared<Label>();
    companyLabel->setText("E&G SIGNS");

    addLayoutElement(
        companyLabel,
        SizePolicy::Fixed,
        35);

    title = std::make_shared<Label>();
    title->setText("Signage Costing System");

    addLayoutElement(
        title,
        SizePolicy::Fixed,
        45);

    subtitleLabel = std::make_shared<Label>();
    subtitleLabel->setText("Professional Costing - Nesting - Production");

    addLayoutElement(
        subtitleLabel,
        SizePolicy::Fixed,
        30);

    bodyPanel = std::make_shared<Panel>();
    bodyPanel->setLayout(std::make_unique<HorizontalLayout>());

    addLayoutElement(
        bodyPanel,
        SizePolicy::Fill);

    navigationPanel = std::make_shared<Panel>();
    navigationPanel->setLayout(std::make_unique<VerticalLayout>());

    statusPanel = std::make_shared<Panel>();
    statusPanel->setLayout(std::make_unique<VerticalLayout>());

    bodyPanel->addLayoutElement(
        navigationPanel,
        SizePolicy::Fixed,
        320);

    bodyPanel->addLayoutElement(
        statusPanel,
        SizePolicy::Fill);

    statusTitle = std::make_shared<Label>();
    statusTitle->setText("System Status");

    statusPanel->addLayoutElement(
        statusTitle,
        SizePolicy::Fixed,
        40);

    materialsStatus = std::make_shared<Label>();
    materialsStatus->setText("[OK] Materials Loaded");

    statusPanel->addLayoutElement(
        materialsStatus,
        SizePolicy::Fixed,
        30);

    pricingStatus = std::make_shared<Label>();
    pricingStatus->setText("[OK] Pricing Loaded");

    statusPanel->addLayoutElement(
        pricingStatus,
        SizePolicy::Fixed,
        30);

    productionStatus = std::make_shared<Label>();
    productionStatus->setText("[OK] Production Pricing Loaded");

    statusPanel->addLayoutElement(
        productionStatus,
        SizePolicy::Fixed,
        30);

    materialCountLabel = std::make_shared<Label>();
    materialCountLabel->setText("Materials : 0");

    statusPanel->addLayoutElement(
        materialCountLabel,
        SizePolicy::Fixed,
        30);

    variantCountLabel = std::make_shared<Label>();
    variantCountLabel->setText("Variants : 0");

    statusPanel->addLayoutElement(
        variantCountLabel,
        SizePolicy::Fixed,
        30);

    newJobButton = std::make_shared<Button>();

    newJobButton->setText("NEW JOB");

    navigationPanel->addLayoutElement(
        newJobButton,
        SizePolicy::Fixed,
        60);

    settingsButton = std::make_shared<Button>();

    settingsButton->setText("SETTINGS");

    navigationPanel->addLayoutElement(
        settingsButton,
        SizePolicy::Fixed,
        60);

    exitButton = std::make_shared<Button>();

    exitButton->setText("EXIT");

    navigationPanel->addLayoutElement(
        exitButton,
        SizePolicy::Fixed,
        60);

    newJobButton->setOnClick([this]()
        {
            if (newJobCallback)
                newJobCallback();
        });

    settingsButton->setOnClick([]()
        {
            std::cout << "Settings clicked\n";
        });

    exitButton->setOnClick([]()
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