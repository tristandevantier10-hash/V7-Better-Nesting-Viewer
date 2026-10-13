#include "LoginScreen.h"

#include "Renderer.h"
#include "VerticalLayout.h"
#include "HorizontalLayout.h"
#include "Metrics.h"
#include "PageHeader.h"

LoginScreen::LoginScreen()
{
    setLayout(std::make_unique<VerticalLayout>());

    setPadding(24);
    setSpacing(12);
    setBackgroundColour(SDL_Color{ 245, 247, 249, 255 });
    setBorderVisible(false);

    //==================================================
    // HEADER
    //==================================================

    auto headerPanel = std::make_shared<PageHeaderPanel>();

    headerPanel->setLayout(
        std::make_unique<HorizontalLayout>());

    configurePageHeader(headerPanel, 8, 8);

    addLayoutElement(
        headerPanel,
        SizePolicy::Fixed,
        100);

    auto titlePanel = std::make_shared<PageHeaderContentPanel>();
    titlePanel->setLayout(std::make_unique<VerticalLayout>());
    configurePageHeaderTitle(titlePanel, 8);
    addPageHeaderLogo(headerPanel);
    headerPanel->addLayoutElement(titlePanel, SizePolicy::Fill);

    titleLabel = std::make_shared<Label>();
    titleLabel->setText("Welcome to EstiMate");
    titleLabel->setStyle(LabelStyle::Heading);
    configurePageHeaderTitleLabel(titleLabel);
    titleLabel->setAlignment(LabelAlignment::Center);

    titlePanel->addLayoutElement(
        titleLabel,
        SizePolicy::Fixed,
        42);

    subtitleLabel = std::make_shared<Label>();
    subtitleLabel->setText(
        "Sign in to access your estimating workspace.");

    subtitleLabel->setStyle(LabelStyle::Small);
    configurePageHeaderSubtitleLabel(subtitleLabel);
    subtitleLabel->setAlignment(LabelAlignment::Center);

    titlePanel->addLayoutElement(
        subtitleLabel,
        SizePolicy::Fixed,
        30);

    //==================================================
    // CENTRED LOGIN PANEL
    //==================================================

    auto bodyPanel = std::make_shared<Panel>();

    bodyPanel->setLayout(
        std::make_unique<HorizontalLayout>());

    bodyPanel->setBorderVisible(false);

    addLayoutElement(
        bodyPanel,
        SizePolicy::Fill);

    auto leftSpacer = std::make_shared<Panel>();
    leftSpacer->setBorderVisible(false);

    auto loginPanel = std::make_shared<Panel>();
    loginPanel->setStyle(PanelStyle::Card);
    loginPanel->setBorderVisible(true);
    loginPanel->setPadding(16);
    loginPanel->setSpacing(8);

    loginPanel->setLayout(
        std::make_unique<VerticalLayout>());

    auto formTitle = std::make_shared<Label>();
    formTitle->setText("Account Login");
    formTitle->setStyle(LabelStyle::Heading);
    loginPanel->addLayoutElement(
        formTitle,
        SizePolicy::Fixed,
        30);

    auto rightSpacer = std::make_shared<Panel>();
    rightSpacer->setBorderVisible(false);

    bodyPanel->addLayoutElement(
        leftSpacer,
        SizePolicy::Fill);

    bodyPanel->addLayoutElement(
        loginPanel,
        SizePolicy::Fixed,
        440);

    bodyPanel->addLayoutElement(
        rightSpacer,
        SizePolicy::Fill);

    //==================================================
    // USERNAME
    //==================================================

    usernameLabel = std::make_shared<Label>();
    usernameLabel->setText("Username");
    usernameLabel->setStyle(LabelStyle::Small);

    loginPanel->addLayoutElement(
        usernameLabel,
        SizePolicy::Fixed,
        20);

    usernameTextBox = std::make_shared<TextBox>();
    usernameTextBox->setPlaceholder("Enter your username");
    usernameTextBox->setSize(360, 40);

    loginPanel->addLayoutElement(
        usernameTextBox,
        SizePolicy::Fixed,
        40);

    //==================================================
    // PASSWORD
    //==================================================

    passwordLabel = std::make_shared<Label>();
    passwordLabel->setText("Password");
    passwordLabel->setStyle(LabelStyle::Small);

    loginPanel->addLayoutElement(
        passwordLabel,
        SizePolicy::Fixed,
        20);

    passwordTextBox = std::make_shared<TextBox>();
    passwordTextBox->setPlaceholder("Enter your password");
    passwordTextBox->setPasswordMode(true);
    passwordTextBox->setSize(360, 40);

    loginPanel->addLayoutElement(
        passwordTextBox,
        SizePolicy::Fixed,
        40);

    //==================================================
    // STATUS MESSAGE
    //==================================================

    statusLabel = std::make_shared<Label>();
    statusLabel->setText("");
    statusLabel->setStyle(LabelStyle::Small);

    loginPanel->addLayoutElement(
        statusLabel,
        SizePolicy::Fixed,
        22);

    //==================================================
    // LOGIN BUTTON
    //==================================================

    loginButton = std::make_shared<Button>();
    loginButton->setText("SIGN IN");

    loginButton->setOnClick(
        [this]()
        {
            if (!loginCallback)
            {
                setStatus(
                    "Login service is not configured.");

                return;
            }

            const std::string username =
                usernameTextBox->getText();

            const std::string password =
                passwordTextBox->getText();

            if (username.empty() || password.empty())
            {
                setStatus(
                    "Enter your username and password.");

                return;
            }

            setStatus("Signing in...");

            loginCallback(username, password);
        });

    loginPanel->addLayoutElement(
        loginButton,
        SizePolicy::Fixed,
        40);
}

void LoginScreen::update(const SDL_Event& e)
{
    Screen::update(e);
}

void LoginScreen::render(Renderer& renderer)
{
    Screen::render(renderer);
}

void LoginScreen::setLoginCallback(
    std::function<void(
        const std::string&,
        const std::string&)> callback)
{
    loginCallback = callback;
}

void LoginScreen::setStatus(
    const std::string& message)
{
    statusMessage = message;

    if (statusLabel)
        statusLabel->setText(statusMessage);
}

void LoginScreen::clearPassword()
{
    if (passwordTextBox)
        passwordTextBox->setText("");
}
