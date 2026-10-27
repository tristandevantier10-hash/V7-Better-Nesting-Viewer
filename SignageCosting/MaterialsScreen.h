#pragma once

#include "Screen.h"
#include "NavigationPanel.h"
#include "DataGrid.h"
#include "TextBox.h"
#include "Label.h"
#include <functional>
#include <memory>

class MaterialsScreen : public Screen
{
public:
    explicit MaterialsScreen(std::shared_ptr<NavigationPanel> sharedNavigation);

    void refreshMaterials();
    void setBackCallback(std::function<void()> callback);

private:
    std::shared_ptr<DataGrid> materialsGrid;
    std::shared_ptr<TextBox> searchBox;
    std::shared_ptr<Label> summaryLabel;
    std::function<void()> backCallback;

    void filterMaterials(const std::string& query);
};
