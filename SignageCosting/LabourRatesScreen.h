#pragma once

#include "Screen.h"
#include <functional>
#include <memory>
#include "ProductionPricingDatabase.h"
#include "PrimaryNavigation.h"

class DataGrid;
class TextBox;
class ComboBox;
class Button;

class LabourRatesScreen : public Screen
{
public:
    LabourRatesScreen();
    void refresh();
    void setBackCallback(std::function<void()> callback);
    void setNavigationCallback(std::function<void(PrimaryNavigationRoute)> callback);

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
    std::function<void(PrimaryNavigationRoute)> navigationCallback;
};
