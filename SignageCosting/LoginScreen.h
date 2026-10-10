#pragma once

#include "Screen.h"
#include "TextBox.h"
#include "Button.h"
#include "Label.h"

#include <functional>
#include <memory>
#include <string>

class LoginScreen : public Screen
{
public:

    LoginScreen();

    void update(const SDL_Event& e) override;

    void render(Renderer& renderer) override;

    void setLoginCallback(
        std::function<void(
            const std::string&,
            const std::string&)> callback);

    void setStatus(const std::string& message);

    void clearPassword();

private:

    std::shared_ptr<Label> titleLabel;
    std::shared_ptr<Label> subtitleLabel;
    std::shared_ptr<Label> usernameLabel;
    std::shared_ptr<Label> passwordLabel;
    std::shared_ptr<Label> statusLabel;

    std::shared_ptr<TextBox> usernameTextBox;
    std::shared_ptr<TextBox> passwordTextBox;

    std::shared_ptr<Button> loginButton;

    std::function<void(
        const std::string&,
        const std::string&)> loginCallback;

    std::string statusMessage;
};
