#include "LoginScreen.h"

#include "Renderer.h"
#include "VerticalLayout.h"
#include "HorizontalLayout.h"
#include "Metrics.h"
#include "PageHeader.h"

namespace
{
    constexpr SDL_Color LoginSurface{ 250, 251, 253, 255 };

    class LoginCardPanel : public Panel
    {
    protected:
        void renderBackground(Renderer& renderer) override
        {
            Panel::renderBackground(renderer);
            renderer.fillRoundedRect(
                { getX() + 16, getY() + 5, 46, 4 },
                SDL_Color{ 0, 170, 90, 255 },
                2);
        }
    };
}

LoginScreen::LoginScreen()
{
    setLayout(std::make_unique<VerticalLayout>());

    setPadding(12);
    setSpacing(8);
    setBackgroundColour(LoginSurface);
    setBorderVisible(false);

    //==================================================
    // CENTRED BRAND HEADER
    //==================================================

    auto headerPanel = std::make_shared<Panel>();

    headerPanel->setLayout(
        std::make_unique<HorizontalLayout>());
    headerPanel->setPadding(0);
    headerPanel->setSpacing(0);
    headerPanel->setBorderVisible(false);
    headerPanel->setBackgroundColour(LoginSurface);

    addLayoutElement(
        headerPanel,
        SizePolicy::Fixed,
        90);

    auto headerLeftSpacer = std::make_shared<Panel>();
    headerLeftSpacer->setBorderVisible(false);
    headerLeftSpacer->setBackgroundColour(LoginSurface);

    auto brandGroup = std::make_shared<PageHeaderPanel>();
    brandGroup->setLayout(std::make_unique<HorizontalLayout>());
    configurePageHeader(brandGroup, 0, 36);
    brandGroup->setBackgroundColour(LoginSurface);
    brandGroup->setBottomAccentVisible(false);

    auto headerRightSpacer = std::make_shared<Panel>();
    headerRightSpacer->setBorderVisible(false);
    headerRightSpacer->setBackgroundColour(LoginSurface);

    headerPanel->addLayoutElement(headerLeftSpacer, SizePolicy::Fill);
    headerPanel->addLayoutElement(brandGroup, SizePolicy::Fixed, 650);
    headerPanel->addLayoutElement(headerRightSpacer, SizePolicy::Fill);

    auto titlePanel = std::make_shared<PageHeaderContentPanel>();
    titlePanel->setLayout(std::make_unique<VerticalLayout>());
    configurePageHeaderTitle(titlePanel, Metrics::SpaceM);
    brandGroup->addLayoutElement(
        std::make_shared<PageHeaderLogo>(),
        SizePolicy::Fixed,
        230);
    brandGroup->addLayoutElement(titlePanel, SizePolicy::Fill);

    titleLabel = std::make_shared<Label>();
    titleLabel->setText("Welcome to EstiMate");
    titleLabel->setStyle(LabelStyle::Heading);
    configurePageHeaderTitleLabel(titleLabel);

    titlePanel->addLayoutElement(
        titleLabel,
        SizePolicy::Fixed,
        38);

    subtitleLabel = std::make_shared<Label>();
    subtitleLabel->setText(
        "Sign in to access your estimating workspace.");

    subtitleLabel->setStyle(LabelStyle::Small);
    configurePageHeaderSubtitleLabel(subtitleLabel);

    titlePanel->addLayoutElement(
        subtitleLabel,
        SizePolicy::Fixed,
        18);

    //==================================================
    // CENTRED LOGIN CARD
    //==================================================

    auto bodyPanel = std::make_shared<Panel>();
    bodyPanel->setLayout(
        std::make_unique<HorizontalLayout>());
    bodyPanel->setPadding(0);
    bodyPanel->setSpacing(0);
    bodyPanel->setBorderVisible(false);
    bodyPanel->setBackgroundColour(LoginSurface);

    addLayoutElement(
        bodyPanel,
        SizePolicy::Fill);

    auto leftSpacer = std::make_shared<Panel>();
    leftSpacer->setBorderVisible(false);
    leftSpacer->setBackgroundColour(LoginSurface);

    auto centerPanel = std::make_shared<Panel>();
    auto centeredLayout = std::make_unique<VerticalLayout>();
    centeredLayout->setCenterContent(true);
    centerPanel->setLayout(std::move(centeredLayout));
    centerPanel->setBorderVisible(false);
    centerPanel->setBackgroundColour(LoginSurface);

    auto loginPanel = std::make_shared<LoginCardPanel>();
    loginPanel->setStyle(PanelStyle::Card);
    loginPanel->setBorderVisible(true);
    loginPanel->setPadding(10);

    auto loginLayout = std::make_unique<VerticalLayout>();
    loginLayout->setSpacing(10);
    loginPanel->setLayout(std::move(loginLayout));

    centerPanel->addLayoutElement(
        loginPanel,
        SizePolicy::Fixed,
        268);

    auto formTitle = std::make_shared<Label>();
    formTitle->setText("Account Login");
    formTitle->setStyle(LabelStyle::Heading);
    loginPanel->addLayoutElement(
        formTitle,
        SizePolicy::Fixed,
        28);

    auto rightSpacer = std::make_shared<Panel>();
    rightSpacer->setBorderVisible(false);
    rightSpacer->setBackgroundColour(LoginSurface);

    bodyPanel->addLayoutElement(
        leftSpacer,
        SizePolicy::Fill);

    bodyPanel->addLayoutElement(
        centerPanel,
        SizePolicy::Fixed,
        520);

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
        18);

    usernameTextBox = std::make_shared<TextBox>();
    usernameTextBox->setPlaceholder("Enter your username");
    usernameTextBox->setSize(360, 36);

    loginPanel->addLayoutElement(
        usernameTextBox,
        SizePolicy::Fixed,
        36);

    //==================================================
    // PASSWORD
    //==================================================

    passwordLabel = std::make_shared<Label>();
    passwordLabel->setText("Password");
    passwordLabel->setStyle(LabelStyle::Small);

    loginPanel->addLayoutElement(
        passwordLabel,
        SizePolicy::Fixed,
        18);

    passwordTextBox = std::make_shared<TextBox>();
    passwordTextBox->setPlaceholder("Enter your password");
    passwordTextBox->setPasswordMode(true);
    passwordTextBox->setSize(360, 36);

    loginPanel->addLayoutElement(
        passwordTextBox,
        SizePolicy::Fixed,
        36);

    //==================================================
    // STATUS MESSAGE
    //==================================================

    statusLabel = std::make_shared<Label>();
    statusLabel->setText("");
    statusLabel->setStyle(LabelStyle::Small);

    loginPanel->addLayoutElement(
        statusLabel,
        SizePolicy::Fixed,
        16);

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
        36);
}

void LoginScreen::update(const SDL_Event& e)
{
    Screen::update(e);
}

void LoginScreen::render(Renderer& renderer)
{
    Screen::render(renderer);

    const SDL_Rect bounds = getBounds();
    renderer.drawRoundedRect(
        { bounds.x + 1, bounds.y + 1, bounds.w - 2, bounds.h - 2 },
        SDL_Color{ 205, 214, 224, 255 },
        9);
    renderer.fillRoundedRect(
        { bounds.x + 2, bounds.y + bounds.h - 5, bounds.w - 4, 4 },
        SDL_Color{ 0, 170, 90, 255 },
        2);
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

void LoginScreen::setWelcomeFont(TTF_Font* font)
{
    if (titleLabel)
        titleLabel->setCustomFont(font);
}

void LoginScreen::clearPassword()
{
    if (passwordTextBox)
        passwordTextBox->setText("");
}
