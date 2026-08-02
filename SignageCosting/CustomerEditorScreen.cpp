#include "CustomerEditorScreen.h"
#include "VerticalLayout.h"
#include "HorizontalLayout.h"
#include "CustomerDatabase.h"
#include "Metrics.h"
#include "NavigationPanel.h"

CustomerEditorScreen::CustomerEditorScreen()
{
    setLayout(std::make_unique<VerticalLayout>());

    //==================================================
// Header
//==================================================

    auto headerPanel = std::make_shared<Panel>();
    headerPanel->setLayout(std::make_unique<HorizontalLayout>());
    headerPanel->setHeaderStyle(true);

    addLayoutElement(
        headerPanel,
        SizePolicy::Fixed,
        Metrics::HeaderHeight);

    auto title = std::make_shared<Label>();
    title->setText("Customer & Invoice");
    title->setStyle(LabelStyle::Heading);

    headerPanel->addLayoutElement(
        title,
        SizePolicy::Fill);

    auto subtitle = std::make_shared<Label>();
    subtitle->setText("Customer Management");
    subtitle->setStyle(LabelStyle::Small);

    headerPanel->addLayoutElement(
        subtitle,
        SizePolicy::Fixed,
        220);

    //==================================================
    // Main Content
    //==================================================

    auto contentPanel = std::make_shared<Panel>();
    contentPanel->setLayout(std::make_unique<HorizontalLayout>());
    contentPanel->setBorderVisible(false);

    addLayoutElement(
        contentPanel,
        SizePolicy::Fill);

    auto sidebarPanel = std::make_shared<NavigationPanel>();
    sidebarPanel->setLayout(std::make_unique<VerticalLayout>());
    sidebarPanel->setSidebarStyle(true);

    sidebarPanel->addItem(
        "CUSTOMER",
        []() {});

    sidebarPanel->addItem(
        "INVOICE",
        []() {});

    sidebarPanel->addItem(
        "SAVE",
        [this]()
        {
            if (saveCallback)
                saveCallback();
        });

    sidebarPanel->addItem(
        "CANCEL",
        [this]()
        {
            if (cancelCallback)
                cancelCallback();
        });

    contentPanel->addLayoutElement(
        sidebarPanel,
        SizePolicy::Fixed,
        Metrics::SidebarWidth);

    auto editorPanel = std::make_shared<Panel>();
    editorPanel->setLayout(std::make_unique<VerticalLayout>());

    contentPanel->addLayoutElement(
        editorPanel,
        SizePolicy::Fill);

    auto companyLabel = std::make_shared<Label>();
    companyLabel->setText("Company");

    editorPanel->addLayoutElement(   
        companyLabel,
        SizePolicy::Fixed,
        25);

    company = std::make_shared<TextBox>();

    editorPanel->addLayoutElement(
        company,
        SizePolicy::Fixed,
        40);

    auto contactLabel = std::make_shared<Label>();
    contactLabel->setText("Contact Person");

    editorPanel->addLayoutElement(
        contactLabel,
        SizePolicy::Fixed,
        25);

    contact = std::make_shared<TextBox>();

    editorPanel->addLayoutElement(
        contact,
        SizePolicy::Fixed,
        40);

    auto phoneLabel = std::make_shared<Label>();
    phoneLabel->setText("Phone");

    editorPanel->addLayoutElement(
        phoneLabel,
        SizePolicy::Fixed,
        25);

    phone = std::make_shared<TextBox>();

    editorPanel->addLayoutElement(
        phone,
        SizePolicy::Fixed,
        40);

    auto emailLabel = std::make_shared<Label>();
    emailLabel->setText("Email");

    editorPanel->addLayoutElement(
        emailLabel,
        SizePolicy::Fixed,
        25);

    email = std::make_shared<TextBox>();

    editorPanel->addLayoutElement(
        email,
        SizePolicy::Fixed,
        40);
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