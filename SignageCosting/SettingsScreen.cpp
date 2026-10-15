#include "SettingsScreen.h"
#include "Button.h"
#include "CardPanel.h"
#include "HorizontalLayout.h"
#include "Label.h"
#include "Metrics.h"
#include "PageHeader.h"
#include "Panel.h"
#include "VerticalLayout.h"
#include <initializer_list>

namespace
{
    std::shared_ptr<Panel> makeSettingRow(
        const std::string& title,
        const std::string& description,
        const std::shared_ptr<ComboBox>& options)
    {
        auto row = std::make_shared<Panel>();
        row->setLayout(std::make_unique<HorizontalLayout>());
        row->setBorderVisible(false);
        row->setPadding(0);
        row->setSpacing(Metrics::SpaceL);

        auto text = std::make_shared<Panel>();
        text->setLayout(std::make_unique<VerticalLayout>());
        text->setBorderVisible(false);
        text->setPadding(0);
        text->setSpacing(Metrics::SpaceXS);

        auto titleLabel = std::make_shared<Label>();
        titleLabel->setText(title);
        titleLabel->setStyle(LabelStyle::Normal);
        text->addLayoutElement(titleLabel, SizePolicy::Fixed, Metrics::NormalRow);

        auto descriptionLabel = std::make_shared<Label>();
        descriptionLabel->setText(description);
        descriptionLabel->setStyle(LabelStyle::Small);
        descriptionLabel->setTextTheme(TextTheme::DarkSecondary);
        text->addLayoutElement(descriptionLabel, SizePolicy::Fixed, Metrics::NormalRow);

        auto optionContainer = std::make_shared<Panel>();
        optionContainer->setLayout(std::make_unique<VerticalLayout>());
        optionContainer->setBorderVisible(false);
        optionContainer->setPadding(15);
        optionContainer->addLayoutElement(options, SizePolicy::Fixed, Metrics::ControlHeight);

        row->addLayoutElement(text, SizePolicy::Fill);
        row->addLayoutElement(optionContainer, SizePolicy::Fixed, 280);
        return row;
    }

    std::shared_ptr<ComboBox> makeOptions(
        const std::initializer_list<const char*>& values)
    {
        auto combo = std::make_shared<ComboBox>();
        for (const char* value : values)
            combo->addItem(value);
        return combo;
    }
}

SettingsScreen::SettingsScreen()
{
    setLayout(std::make_unique<VerticalLayout>());

    auto header = std::make_shared<PageHeaderPanel>();
    header->setLayout(std::make_unique<HorizontalLayout>());
    configurePageHeader(header);
    addPageHeaderLogo(header);

    auto title = std::make_shared<Label>();
    title->setText("Settings");
    title->setStyle(LabelStyle::Heading);
    configurePageHeaderTitleLabel(title);
    title->setAlignment(LabelAlignment::Center);
    header->addLayoutElement(title, SizePolicy::Fill);

    auto backButton = std::make_shared<Button>();
    backButton->setText("Back");
    backButton->setOnClick([this]()
    {
        if (backCallback)
            backCallback();
    });
    header->addLayoutElement(backButton, SizePolicy::Fixed, 100);
    addLayoutElement(header, SizePolicy::Fixed, Metrics::HeaderHeight);

    auto pageContent = std::make_shared<Panel>();
    pageContent->setLayout(std::make_unique<HorizontalLayout>());
    addLayoutElement(pageContent, SizePolicy::Fill);

    auto navigation = createPrimaryNavigationPanel(
        PrimaryNavigationRoute::Settings,
        [this](PrimaryNavigationRoute route)
        {
            if (navigationCallback)
                navigationCallback(route);
        });
    pageContent->addLayoutElement(
        navigation,
        SizePolicy::Fixed,
        Metrics::SidebarWidth);

    auto body = std::make_shared<Panel>();
    body->setLayout(std::make_unique<VerticalLayout>());
    body->setBorderVisible(false);
    body->setPadding(Metrics::SpaceXL);
    body->setSpacing(Metrics::SpaceL);
    pageContent->addLayoutElement(body, SizePolicy::Fill);

    auto introduction = std::make_shared<Label>();
    introduction->setText("Configure the default layout and typography for your workspace.");
    introduction->setStyle(LabelStyle::Small);
    introduction->setTextTheme(TextTheme::DarkSecondary);
    body->addLayoutElement(introduction, SizePolicy::Fixed, Metrics::LargeRow);

    auto layoutCard = std::make_shared<CardPanel>();
    layoutCard->setTitle("Layout");
    layoutCard->setSpacing(Metrics::SpaceS);
    layoutCard->addLayoutElement(
        makeSettingRow(
            "Display density",
            "Choose the spacing used across screens.",
            layoutDensity = makeOptions({ "Comfortable", "Compact" })),
        SizePolicy::Fixed,
        64);
    layoutCard->addLayoutElement(
        makeSettingRow(
            "Window size",
            "Choose how the workspace opens.",
            windowSize = makeOptions({ "Automatic", "1280 x 800", "1600 x 900" })),
        SizePolicy::Fixed,
        64);
    body->addLayoutElement(layoutCard, SizePolicy::Fixed, 232);

    auto fontsCard = std::make_shared<CardPanel>();
    fontsCard->setTitle("Fonts");
    fontsCard->setSpacing(Metrics::SpaceS);
    fontsCard->addLayoutElement(
        makeSettingRow(
            "Font family",
            "Select the typeface used throughout the app.",
            fontFamily = makeOptions({ "Basis Grotesque", "System default" })),
        SizePolicy::Fixed,
        64);
    fontsCard->addLayoutElement(
        makeSettingRow(
            "Text size",
            "Adjust the default interface text scale.",
            textSize = makeOptions({ "Default", "Larger" })),
        SizePolicy::Fixed,
        64);
    body->addLayoutElement(fontsCard, SizePolicy::Fixed, 232);

    auto actions = std::make_shared<Panel>();
    actions->setLayout(std::make_unique<HorizontalLayout>());
    actions->setBorderVisible(false);
    actions->setPadding(0);

    auto resetButton = std::make_shared<Button>();
    resetButton->setText("Reset defaults");
    resetButton->setOnClick([this]() { resetToDefaults(); });
    actions->addLayoutElement(resetButton, SizePolicy::Fixed, 150);
    body->addLayoutElement(actions, SizePolicy::Fixed, Metrics::LargeRow);
}

void SettingsScreen::setBackCallback(std::function<void()> callback)
{
    backCallback = callback;
}

void SettingsScreen::setNavigationCallback(
    std::function<void(PrimaryNavigationRoute)> callback)
{
    navigationCallback = callback;
}

void SettingsScreen::resetToDefaults()
{
    layoutDensity->setSelectedIndex(0);
    windowSize->setSelectedIndex(0);
    fontFamily->setSelectedIndex(0);
    textSize->setSelectedIndex(0);
}
