#pragma once

#include "Screen.h"
#include <functional>
#include <memory>
#include "ProductionPricingDatabase.h"
#include "NavigationPanel.h"

class DataGrid;
class TextBox;
class ComboBox;
class Button;

class LabourRatesScreen : public Screen
{
public:
    explicit LabourRatesScreen(std::shared_ptr<NavigationPanel> sharedNavigation);
    void refresh();
    void setBackCallback(std::function<void()> callback);

private:
    void loadSelected(int row);
    void saveProcess();
    void toggleEnabled();
    std::shared_ptr<DataGrid> grid;
    std::shared_ptr<TextBox> name;
    std::shared_ptr<TextBox> rate;
    std::shared_ptr<ComboBox> unit;
    std::shared_ptr<Button> saveButton;
    std::shared_ptr<Button> enabledButton;
    std::string editingId;
    std::function<void()> backCallback;
};
