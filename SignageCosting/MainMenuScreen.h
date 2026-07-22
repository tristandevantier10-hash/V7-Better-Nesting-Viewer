#pragma once

#include "Screen.h"
#include "Button.h"
#include "Label.h"
#include <functional>

class Renderer;

class MainMenuScreen : public Screen
{
public:

    MainMenuScreen();

    void setNewJobCallback(std::function<void()> callback);

    void update(const SDL_Event& e) override;

    void render(Renderer& renderer) override;

    void refreshStatus();

private:

    std::shared_ptr<Panel> bodyPanel;

    std::shared_ptr<Panel> navigationPanel;

    std::shared_ptr<Panel> statusPanel;

    std::shared_ptr<Label> companyLabel;

    std::shared_ptr<Label> subtitleLabel;

    std::shared_ptr<Label> title;

    std::shared_ptr<Label> statusTitle;

    std::shared_ptr<Label> materialsStatus;

    std::shared_ptr<Label> pricingStatus;

    std::shared_ptr<Label> productionStatus;

    std::shared_ptr<Label> materialCountLabel;

    std::shared_ptr<Label> variantCountLabel;

    std::shared_ptr<Button> newJobButton;

    std::shared_ptr<Button> settingsButton;

    std::shared_ptr<Button> exitButton;

    std::function<void()> newJobCallback;
};