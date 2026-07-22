#include "CustomerSelectionScreen.h"
#include "VerticalLayout.h"
#include "ListView.h"
#include "CustomerDatabase.h"

CustomerSelectionScreen::CustomerSelectionScreen()
{
    setLayout(std::make_unique<VerticalLayout>());

    title = std::make_shared<Label>();
    title->setText("SELECT CUSTOMER");

    addLayoutElement(
        title,
        SizePolicy::Fixed,
        50);

    searchBox = std::make_shared<TextBox>();

    addLayoutElement(
        searchBox,
        SizePolicy::Fixed,
        40);

    customerList = std::make_shared<ListView>();

    addLayoutElement(
        customerList,
        SizePolicy::Fill);

    newCustomerButton = std::make_shared<Button>();
    newCustomerButton->setText("NEW CUSTOMER");

    addLayoutElement(
        newCustomerButton,
        SizePolicy::Fixed,
        50);

    editButton = std::make_shared<Button>();
    editButton->setText("EDIT CUSTOMER");

    addLayoutElement(
        editButton,
        SizePolicy::Fixed,
        50);

    deleteButton = std::make_shared<Button>();
    deleteButton->setText("DELETE CUSTOMER");

    addLayoutElement(
        deleteButton,
        SizePolicy::Fixed,
        50);

    selectButton = std::make_shared<Button>();
    selectButton->setText("SELECT CUSTOMER");

    addLayoutElement(
        selectButton,
        SizePolicy::Fixed,
        50);

    newCustomerButton->setOnClick([this]()
        {
            if (newCustomerCallback)
                newCustomerCallback();
        });

    editButton->setOnClick([this]()
        {
            if (editCustomerCallback)
            {
                editCustomerCallback(
                    customerList->getSelectedIndex());
            }
        });

    deleteButton->setOnClick([this]()
        {
            if (deleteCustomerCallback)
            {
                deleteCustomerCallback(
                    customerList->getSelectedIndex());
            }
        });

    selectButton->setOnClick([this]()
        {
            if (selectCustomerCallback)
            {
                selectCustomerCallback(
                    customerList->getSelectedIndex());
            }
        });
}

void CustomerSelectionScreen::setNewCustomerCallback(std::function<void()> callback)
{
    newCustomerCallback = callback;
}

void CustomerSelectionScreen::setSelectCustomerCallback(
    std::function<void(int)> callback)
{
    selectCustomerCallback = callback;
}

void CustomerSelectionScreen::refreshCustomers()
{
    customerList->clear();

    const auto& customers =
        CustomerDatabase::getAll();

    for (const auto& customer : customers)
    {
        customerList->addItem(
            customer.company + " - " + customer.contact);
    }
}

void CustomerSelectionScreen::setEditCustomerCallback(
    std::function<void(int)> callback)
{
    editCustomerCallback = callback;
}

void CustomerSelectionScreen::setDeleteCustomerCallback(
    std::function<void(int)> callback)
{
    deleteCustomerCallback = callback;
}