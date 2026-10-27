#pragma once

#include "Screen.h"
#include "ComboBox.h"
#include "NavigationPanel.h"
#include <functional>
#include <memory>

class SettingsScreen : public Screen
{
public:
    explicit SettingsScreen(std::shared_ptr<NavigationPanel> sharedNavigation);

    void setBackCallback(std::function<void()> callback);

private:
    std::shared_ptr<ComboBox> layoutDensity;
    std::shared_ptr<ComboBox> windowSize;
    std::shared_ptr<ComboBox> fontFamily;
    std::shared_ptr<ComboBox> textSize;
    std::function<void()> backCallback;

    void resetToDefaults();
};
