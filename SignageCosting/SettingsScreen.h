#pragma once

#include "Screen.h"
#include "ComboBox.h"
#include "PrimaryNavigation.h"
#include <functional>
#include <memory>

class SettingsScreen : public Screen
{
public:
    SettingsScreen();

    void setBackCallback(std::function<void()> callback);
    void setNavigationCallback(
        std::function<void(PrimaryNavigationRoute)> callback);

private:
    std::shared_ptr<ComboBox> layoutDensity;
    std::shared_ptr<ComboBox> windowSize;
    std::shared_ptr<ComboBox> fontFamily;
    std::shared_ptr<ComboBox> textSize;
    std::function<void()> backCallback;
    std::function<void(PrimaryNavigationRoute)> navigationCallback;

    void resetToDefaults();
};
