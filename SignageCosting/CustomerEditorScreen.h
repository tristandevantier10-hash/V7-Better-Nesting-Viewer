#pragma once

#include "Screen.h"
#include "TextBox.h"
#include "Button.h"
#include "Label.h"
#include <memory>
#include <functional>
#include "Customer.h"

class CustomerEditorScreen : public Screen
{
public:

    CustomerEditorScreen();

    void editCustomer(
        int index,
        const Customer& customer);

    void clearEditor();

    void setSaveCallback(std::function<void()> callback);

    void setCancelCallback(std::function<void()> callback);

    Customer createCustomer() const;

    bool isEditMode() const;

    int getEditingIndex() const;

private:

    bool editMode = false;
    int editingIndex = -1;

    std::shared_ptr<TextBox> company;
    std::shared_ptr<TextBox> contact;
    std::shared_ptr<TextBox> phone;
    std::shared_ptr<TextBox> email;

    std::shared_ptr<Button> saveButton;
    std::shared_ptr<Button> cancelButton;

    std::function<void()> saveCallback;
    std::function<void()> cancelCallback;

};