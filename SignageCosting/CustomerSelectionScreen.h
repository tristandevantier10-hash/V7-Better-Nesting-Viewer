#pragma once

#include "Screen.h"
#include "Panel.h"
#include "Button.h"
#include "ListView.h"
#include "TextBox.h"
#include "Label.h"
#include <memory>
#include <functional>


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

    void refreshCustomers();

private:

    std::shared_ptr<Label> title;

    std::shared_ptr<TextBox> searchBox;

    std::shared_ptr<ListView> customerList;

    std::shared_ptr<Button> newCustomerButton;

    std::shared_ptr<Button> editButton;

    std::shared_ptr<Button> deleteButton;

    std::shared_ptr<Button> selectButton;

    std::function<void()> newCustomerCallback;

    std::function<void(int)> selectCustomerCallback;

    std::function<void(int)> editCustomerCallback;

    std::function<void(int)> deleteCustomerCallback;

};
