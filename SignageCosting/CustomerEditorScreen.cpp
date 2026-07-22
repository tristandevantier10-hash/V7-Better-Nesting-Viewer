#include "CustomerEditorScreen.h"
#include "VerticalLayout.h"
#include "HorizontalLayout.h"
#include "CustomerDatabase.h"

CustomerEditorScreen::CustomerEditorScreen()
{
    setLayout(std::make_unique<VerticalLayout>());

    auto title = std::make_shared<Label>();
    title->setText("NEW CUSTOMER");

    addLayoutElement(
        title,
        SizePolicy::Fixed,
        50);

    auto companyLabel = std::make_shared<Label>();
    companyLabel->setText("Company");

    addLayoutElement(
        companyLabel,
        SizePolicy::Fixed,
        25);

    company = std::make_shared<TextBox>();

    addLayoutElement(
        company,
        SizePolicy::Fixed,
        40);

    auto contactLabel = std::make_shared<Label>();
    contactLabel->setText("Contact Person");

    addLayoutElement(
        contactLabel,
        SizePolicy::Fixed,
        25);

    contact = std::make_shared<TextBox>();

    addLayoutElement(
        contact,
        SizePolicy::Fixed,
        40);

    auto phoneLabel = std::make_shared<Label>();
    phoneLabel->setText("Phone");

    addLayoutElement(
        phoneLabel,
        SizePolicy::Fixed,
        25);

    phone = std::make_shared<TextBox>();

    addLayoutElement(
        phone,
        SizePolicy::Fixed,
        40);

    auto emailLabel = std::make_shared<Label>();
    emailLabel->setText("Email");

    addLayoutElement(
        emailLabel,
        SizePolicy::Fixed,
        25);

    email = std::make_shared<TextBox>();

    addLayoutElement(
        email,
        SizePolicy::Fixed,
        40);

    saveButton = std::make_shared<Button>();
    saveButton->setText("SAVE");

    cancelButton = std::make_shared<Button>();
    cancelButton->setText("CANCEL");

    auto buttonPanel = std::make_shared<Panel>();
    buttonPanel->setLayout(std::make_unique<HorizontalLayout>());

    buttonPanel->addLayoutElement(
        saveButton,
        SizePolicy::Fill);

    buttonPanel->addLayoutElement(
        cancelButton,
        SizePolicy::Fill);

    addLayoutElement(
        buttonPanel,
        SizePolicy::Fixed,
        50);

    saveButton->setOnClick([this]()
        {
            if (saveCallback)
                saveCallback();
        });

    cancelButton->setOnClick([this]()
        {
            if (cancelCallback)
                cancelCallback();
        });
}

void CustomerEditorScreen::setSaveCallback(std::function<void()> callback)
{
    saveCallback = callback;
}

void CustomerEditorScreen::setCancelCallback(std::function<void()> callback)
{
    cancelCallback = callback;
}

Customer CustomerEditorScreen::createCustomer() const
{
    Customer customer;

    customer.company = company->getText();

    customer.contact = contact->getText();

    customer.phone = phone->getText();

    customer.email = email->getText();

    return customer;
}

void CustomerEditorScreen::clearEditor()
{
    company->setText("");

    contact->setText("");

    phone->setText("");

    email->setText("");

    editMode = false;

    editingIndex = -1;
}

void CustomerEditorScreen::editCustomer(
    int index,
    const Customer& customer)
{
    company->setText(customer.company);

    contact->setText(customer.contact);

    phone->setText(customer.phone);

    email->setText(customer.email);

    editMode = true;

    editingIndex = index;
}

bool CustomerEditorScreen::isEditMode() const
{
    return editMode;
}

int CustomerEditorScreen::getEditingIndex() const
{
    return editingIndex;
}