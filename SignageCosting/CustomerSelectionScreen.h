#pragma once

#include "Screen.h"
#include "Panel.h"
#include "Button.h"
#include "DataGrid.h"
#include "TextBox.h"
#include "Label.h"
#include <memory>
#include <functional>
#include "Customer.h"    // for AccountType
#include "SegmentedControl.h"

class CustomerSelectionScreen : public Screen
{
public:

    CustomerSelectionScreen();

    void setNewCustomerCallback(std::function<void()> callback);

    void setSelectCustomerCallback(
        std::function<void(int)> callback);

    void setEditCustomerCallback(
        std::function<void(int)> callback);

    void setDeleteCustomerCallback(
        std::function<void(int)> callback);

    void setBackCallback(std::function<void()> callback);

    void refreshCustomers();

private:

    std::shared_ptr<Label> title;

    std::shared_ptr<TextBox> searchBox;

    std::shared_ptr<SegmentedControl> accountSelector;

    AccountType currentAccountType = AccountType::Cash;

    std::shared_ptr<DataGrid> customerGrid;

    std::shared_ptr<Button> newCustomerButton;

    std::shared_ptr<Button> editButton;

    std::shared_ptr<Button> deleteButton;

    std::shared_ptr<Button> selectButton;

    std::function<void()> newCustomerCallback;

    std::function<void(int)> selectCustomerCallback;

    std::function<void(int)> editCustomerCallback;

    std::function<void(int)> deleteCustomerCallback;

    std::function<void()> backCallback;

};
