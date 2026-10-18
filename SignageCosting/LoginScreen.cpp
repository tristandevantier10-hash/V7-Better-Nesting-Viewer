#include "LoginScreen.h"

#include "Renderer.h"
#include "VerticalLayout.h"
#include "HorizontalLayout.h"
#include "Metrics.h"
#include "PageHeader.h"

namespace
{
    constexpr SDL_Color LoginSurface{ 255, 255, 255, 255 };

    class LoginHeadingBanner : public Panel
    {
    protected:
        void renderBackground(Renderer& renderer) override
        {
            renderer.fillRect(getBounds(), backgroundColour);
        }
    };
}

LoginScreen::LoginScreen()
{
    setLayout(std::make_unique<VerticalLayout>());
    setPadding(0);
    setSpacing(0);
    setBackgroundColour(LoginSurface);
    setBorderVisible(false);

    // Center the brand and sign-in form in a compact, Slack-inspired layout.
    auto bodyPanel = std::make_shared<Panel>();
    bodyPanel->setLayout(std::make_unique<HorizontalLayout>());
    bodyPanel->setPadding(0);
    bodyPanel->setSpacing(0);
    bodyPanel->setBorderVisible(false);
    bodyPanel->setBackgroundColour(LoginSurface);
    addLayoutElement(bodyPanel, SizePolicy::Fill);

    auto centerPanel = std::make_shared<Panel>();
    auto centeredLayout = std::make_unique<VerticalLayout>();
    centeredLayout->setCenterContent(true);
    // Keep the sign-in stack high enough to leave room for a footer disclaimer.
    centeredLayout->setTopInset(-100);
    centeredLayout->setSpacing(15);
    centerPanel->setLayout(std::move(centeredLayout));
    centerPanel->setBorderVisible(false);
    centerPanel->setBackgroundColour(LoginSurface);

    bodyPanel->addLayoutElement(centerPanel, SizePolicy::Fill);

    auto brandPanel = std::make_shared<Panel>();
    brandPanel->setLayout(std::make_unique<HorizontalLayout>());
    brandPanel->setPadding(0);
    brandPanel->setSpacing(0);
    brandPanel->setBorderVisible(false);
    brandPanel->setBackgroundColour(LoginSurface);

    auto brandLeftSpacer = std::make_shared<Panel>();
    brandLeftSpacer->setBorderVisible(false);
    brandLeftSpacer->setBackgroundColour(LoginSurface);
    auto brandRightSpacer = std::make_shared<Panel>();
    brandRightSpacer->setBorderVisible(false);
    brandRightSpacer->setBackgroundColour(LoginSurface);

    brandPanel->addLayoutElement(brandLeftSpacer, SizePolicy::Fill);
    brandPanel->addLayoutElement(
        std::make_shared<PageHeaderLogo>(),
        SizePolicy::Fixed,
        300);
    brandPanel->addLayoutElement(brandRightSpacer, SizePolicy::Fill);
    centerPanel->addLayoutElement(brandPanel, SizePolicy::Fixed, 100);

    auto headingBanner = std::make_shared<LoginHeadingBanner>();
    auto headingLayout = std::make_unique<VerticalLayout>();
    headingLayout->setCenterContent(true);
    headingLayout->setSpacing(2);
    headingBanner->setLayout(std::move(headingLayout));
    headingBanner->setPadding(10);
    headingBanner->setBorderVisible(false);
    headingBanner->setBackgroundColour(SDL_Color{ 20, 20, 20, 255 });

    titleLabel = std::make_shared<Label>();
    titleLabel->setText("Sign in to EstiMate");
    titleLabel->setStyle(LabelStyle::Heading);
    titleLabel->setTextTheme(TextTheme::Light);
    titleLabel->setAlignment(LabelAlignment::Center);
    headingBanner->addLayoutElement(titleLabel, SizePolicy::Fixed, 34);

    subtitleLabel = std::make_shared<Label>();
    subtitleLabel->setText("Use your company account to continue.");
    subtitleLabel->setStyle(LabelStyle::Small);
    subtitleLabel->setTextTheme(TextTheme::LightSecondary);
    subtitleLabel->setAlignment(LabelAlignment::Center);
    headingBanner->addLayoutElement(subtitleLabel, SizePolicy::Fixed, 22);
    centerPanel->addLayoutElement(headingBanner, SizePolicy::Fixed, 78);

    auto loginPanel = std::make_shared<Panel>();
    loginPanel->setBorderVisible(false);
    loginPanel->setBackgroundColour(LoginSurface);
    loginPanel->setPadding(24);

    auto loginLayout = std::make_unique<VerticalLayout>();
    loginLayout->setSpacing(15);
    loginPanel->setLayout(std::move(loginLayout));

    auto loginCardRow = std::make_shared<Panel>();
    loginCardRow->setLayout(std::make_unique<HorizontalLayout>());
    loginCardRow->setPadding(0);
    loginCardRow->setSpacing(0);
    loginCardRow->setBorderVisible(false);
    loginCardRow->setBackgroundColour(LoginSurface);
    auto cardLeftSpacer = std::make_shared<Panel>();
    cardLeftSpacer->setBorderVisible(false);
    cardLeftSpacer->setBackgroundColour(LoginSurface);
    auto cardRightSpacer = std::make_shared<Panel>();
    cardRightSpacer->setBorderVisible(false);
    cardRightSpacer->setBackgroundColour(LoginSurface);
    loginCardRow->addLayoutElement(cardLeftSpacer, SizePolicy::Fill);
    loginCardRow->addLayoutElement(loginPanel, SizePolicy::Fixed, 440);
    loginCardRow->addLayoutElement(cardRightSpacer, SizePolicy::Fill);
    centerPanel->addLayoutElement(loginCardRow, SizePolicy::Fixed, 270);

    auto formTitle = std::make_shared<Label>();
    formTitle->setText("SIGN IN WITH YOUR ACCOUNT");
    formTitle->setStyle(LabelStyle::Normal);
    formTitle->setTextTheme(TextTheme::Dark);
    formTitle->setAlignment(LabelAlignment::Center);
    loginPanel->addLayoutElement(
        formTitle,
        SizePolicy::Fixed,
        28);

    //==================================================
    // USERNAME
    //==================================================

    usernameLabel = std::make_shared<Label>();
    usernameLabel->setText("Username");
    usernameLabel->setStyle(LabelStyle::Small);
    usernameLabel->setTextTheme(TextTheme::DarkSecondary);
    usernameLabel->setAlignment(LabelAlignment::Center);

    loginPanel->addLayoutElement(
        usernameLabel,
        SizePolicy::Fixed,
        14);

    usernameTextBox = std::make_shared<TextBox>();
    usernameTextBox->setPlaceholder("Enter your username");
    usernameTextBox->setSize(360, 42);
    usernameTextBox->setTextCentered(true);

    loginPanel->addLayoutElement(
        usernameTextBox,
        SizePolicy::Fixed,
        42);

    //==================================================
    // PASSWORD
    //==================================================

    passwordLabel = std::make_shared<Label>();
    passwordLabel->setText("Password");
    passwordLabel->setStyle(LabelStyle::Small);
    passwordLabel->setTextTheme(TextTheme::DarkSecondary);
    passwordLabel->setAlignment(LabelAlignment::Center);

    loginPanel->addLayoutElement(
        passwordLabel,
        SizePolicy::Fixed,
        14);

    passwordTextBox = std::make_shared<TextBox>();
    passwordTextBox->setPlaceholder("Enter your password");
    passwordTextBox->setPasswordMode(true);
    passwordTextBox->setSize(360, 42);
    passwordTextBox->setTextCentered(true);

    loginPanel->addLayoutElement(
        passwordTextBox,
        SizePolicy::Fixed,
        42);

    //==================================================
    // LOGIN BUTTON
    //==================================================

    loginButton = std::make_shared<Button>();
    loginButton->setText("SIGN IN");
    loginButton->setStyle(Button::ButtonStyle::Primary);
    loginButton->setTextCentered(true);

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
        42);
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

    if (subtitleLabel)
    {
        subtitleLabel->setText(
            message.empty()
            ? "Use your company account to continue."
            : statusMessage);

        if (message.empty() || message == "Signing in...")
            subtitleLabel->setTextTheme(TextTheme::LightSecondary);
        else if (message.find("successful") != std::string::npos ||
            message.find("accepted") != std::string::npos)
            subtitleLabel->setTextTheme(TextTheme::Success);
        else
            subtitleLabel->setTextTheme(TextTheme::Error);
    }
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
