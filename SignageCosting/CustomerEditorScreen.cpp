#include "CustomerEditorScreen.h"
#include "VerticalLayout.h"
#include "HorizontalLayout.h"
#include "CustomerDatabase.h"
#include "Metrics.h"
#include "NavigationPanel.h"
#include "Panel.h"
#include "Label.h"
#include "TextBox.h"

CustomerEditorScreen::CustomerEditorScreen()
{
    //==================================================
    // Root Layout
    //==================================================
    setLayout(std::make_unique<VerticalLayout>());

    auto contentPanel = std::make_shared<Panel>();
    contentPanel->setLayout(std::make_unique<HorizontalLayout>());

    addLayoutElement(contentPanel, SizePolicy::Fill);

    //==================================================
    // Sidebar
    //==================================================
    auto sidebarPanel = std::make_shared<NavigationPanel>();
    sidebarPanel->setLayout(std::make_unique<VerticalLayout>());
    sidebarPanel->setSidebarStyle(true);
    sidebarPanel->setPadding(Metrics::SidebarPadding);

    contentPanel->addLayoutElement(
        sidebarPanel,
        SizePolicy::Fixed,
        Metrics::SidebarWidth);

    sidebarPanel->addItem("CUSTOMER", []() {});
    sidebarPanel->addItem("INVOICE", []() {});

    // Invisible Spacer matching Selection Screen theme
    struct InvisibleSpacer : public Panel
    {
        void render(Renderer& renderer) override
        {
            performLayout();
            renderChildren(renderer);
        }
    };

    auto spacer = std::make_shared<InvisibleSpacer>();
    spacer->setBorderVisible(false);
    sidebarPanel->addLayoutElement(spacer, SizePolicy::Fill);

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

    //==================================================
    // Right Side Container
    //==================================================
    auto rightContainer = std::make_shared<Panel>();
    rightContainer->setLayout(std::make_unique<VerticalLayout>());
    rightContainer->setStyle(PanelStyle::Card);
    rightContainer->setBorderVisible(false);
    rightContainer->setPadding(30);
    rightContainer->setSpacing(20);

    contentPanel->addLayoutElement(
        rightContainer,
        SizePolicy::Fill);

    //==================================================
    // Page Header (Integrated into Main Area)
    //==================================================
    auto pageHeader = std::make_shared<Panel>();
    pageHeader->setLayout(std::make_unique<HorizontalLayout>());
    pageHeader->setStyle(PanelStyle::Card);
    pageHeader->setBorderVisible(false);

    rightContainer->addLayoutElement(
        pageHeader,
        SizePolicy::Fixed,
        80);

    auto titlePanel = std::make_shared<Panel>();
    titlePanel->setLayout(std::make_unique<VerticalLayout>());
    titlePanel->setStyle(PanelStyle::Card);
    titlePanel->setBorderVisible(false);
    titlePanel->setSpacing(4);

    pageHeader->addLayoutElement(
        titlePanel,
        SizePolicy::Fill);

    auto title = std::make_shared<Label>();
    title->setText("Customer & Invoice");
    title->setStyle(LabelStyle::Heading);

    titlePanel->addLayoutElement(
        title,
        SizePolicy::Fixed,
        36);

    auto subtitle = std::make_shared<Label>();
    subtitle->setText("Customer Management");
    subtitle->setStyle(LabelStyle::Small);
    subtitle->setTextTheme(TextTheme::DarkSecondary);

    titlePanel->addLayoutElement(
        subtitle,
        SizePolicy::Fixed,
        220);

    //==================================================
    // Editor Form Panel
    //==================================================
    auto editorPanel = std::make_shared<Panel>();
    editorPanel->setLayout(std::make_unique<VerticalLayout>());
    editorPanel->setStyle(PanelStyle::Card); // Maintains uniform background formatting
    editorPanel->setBorderVisible(false);
    editorPanel->setSpacing(10); // Standard clean item distribution gap

    rightContainer->addLayoutElement(
        editorPanel,
        SizePolicy::Fill);

    // Form Field: Company
    auto companyLabel = std::make_shared<Label>();
    companyLabel->setText("Company");
    companyLabel->setStyle(LabelStyle::Small);
    companyLabel->setTextTheme(TextTheme::DarkSecondary);
    editorPanel->addLayoutElement(companyLabel, SizePolicy::Fixed, 25);

    company = std::make_shared<TextBox>();
    editorPanel->addLayoutElement(company, SizePolicy::Fixed, 40);

    // Form Field: Contact Person
    auto contactLabel = std::make_shared<Label>();
    contactLabel->setText("Contact Person");
    contactLabel->setStyle(LabelStyle::Small);
    contactLabel->setTextTheme(TextTheme::DarkSecondary);
    editorPanel->addLayoutElement(contactLabel, SizePolicy::Fixed, 25);

    contact = std::make_shared<TextBox>();
    editorPanel->addLayoutElement(contact, SizePolicy::Fixed, 40);

    // Form Field: Phone
    auto phoneLabel = std::make_shared<Label>();
    phoneLabel->setText("Phone");
    phoneLabel->setStyle(LabelStyle::Small);
    phoneLabel->setTextTheme(TextTheme::DarkSecondary);
    editorPanel->addLayoutElement(phoneLabel, SizePolicy::Fixed, 25);

    phone = std::make_shared<TextBox>();
    editorPanel->addLayoutElement(phone, SizePolicy::Fixed, 40);

    // Form Field: Email
    auto emailLabel = std::make_shared<Label>();
    emailLabel->setText("Email");
    emailLabel->setStyle(LabelStyle::Small);
    emailLabel->setTextTheme(TextTheme::DarkSecondary);
    editorPanel->addLayoutElement(emailLabel, SizePolicy::Fixed, 25);

    email = std::make_shared<TextBox>();
    editorPanel->addLayoutElement(email, SizePolicy::Fixed, 40);
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

void CustomerEditorScreen::editCustomer(int index, const Customer& customer)
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