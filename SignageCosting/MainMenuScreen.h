#pragma once

#include "Screen.h"
#include "Label.h"
#include <functional>
#include "NavigationPanel.h"
#include "DashboardContent.h"

class Renderer;
class NavigationItem;

class MainMenuScreen : public Screen
{
public:

    MainMenuScreen();
    std::shared_ptr<NavigationPanel> getNavigationPanel() const { return sidebarPanel; }
    void onActivated() override;

    void setNewJobCallback(std::function<void()> callback);
    void setQuotesCallback(std::function<void()> callback);
    void setCustomerDatabaseCallback(std::function<void()> callback);
    void setMaterialsCallback(std::function<void()> callback);
    void setSettingsCallback(std::function<void()> callback);
    void setLabourRatesCallback(std::function<void()> callback);
    void setLabourRatesAvailable(bool available);
    void setExitCallback(std::function<void()> callback);
    void setOverviewCallback(std::function<void()> callback);

    void update(const SDL_Event& e) override;

    void render(Renderer& renderer) override;

    void refreshStatus();

private:

    std::shared_ptr<Panel> contentPanel;

    std::shared_ptr<NavigationPanel> sidebarPanel;

    std::shared_ptr<DashboardContent> dashboardContent;
    std::shared_ptr<NavigationItem> overviewItem;
    std::shared_ptr<NavigationItem> labourRatesItem;

    std::function<void()> newJobCallback;
    std::function<void()> quotesCallback;
    std::function<void()> customerDatabaseCallback;
    std::function<void()> materialsCallback;
    std::function<void()> settingsCallback;
    std::function<void()> labourRatesCallback;
    std::function<void()> exitCallback;
    std::function<void()> overviewCallback;
};
