#include "CustomerEditorScreen.h"
#include "VerticalLayout.h"
#include "HorizontalLayout.h"
#include "CustomerDatabase.h"
#include "Metrics.h"
#include "PageHeader.h"
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

    sidebarPanel->addHeading("CUSTOMER");
    sidebarPanel->addItem("INVOICE", {}, "", false);

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
    rightContainer->setPadding(Metrics::SpaceXL);
    rightContainer->setSpacing(Metrics::SpaceL);

    contentPanel->addLayoutElement(
        rightContainer,
        SizePolicy::Fill);

    //==================================================
    // Page Header (Integrated into Main Area)
    //==================================================
    auto pageHeader = std::make_shared<PageHeaderPanel>();
    pageHeader->setLayout(std::make_unique<HorizontalLayout>());
    configurePageHeader(pageHeader);

    rightContainer->addLayoutElement(
        pageHeader,
        SizePolicy::Fixed,
        Metrics::HeaderHeight);

    auto titlePanel = std::make_shared<PageHeaderContentPanel>();
    titlePanel->setLayout(std::make_unique<VerticalLayout>());
    configurePageHeaderTitle(titlePanel);
    addPageHeaderLogo(pageHeader);

    pageHeader->addLayoutElement(
        titlePanel,
        SizePolicy::Fill);

    auto title = std::make_shared<Label>();
    title->setText("Customer & Invoice");
    title->setStyle(LabelStyle::Heading);
    configurePageHeaderTitleLabel(title);
    title->setAlignment(LabelAlignment::Center);

    titlePanel->addLayoutElement(
        title,
        SizePolicy::Fixed,
        36);

    auto subtitle = std::make_shared<Label>();
    subtitle->setText("Customer Management");
    subtitle->setStyle(LabelStyle::Small);
    configurePageHeaderSubtitleLabel(subtitle);
    subtitle->setAlignment(LabelAlignment::Center);

    titlePanel->addLayoutElement(
        subtitle,
        SizePolicy::Fixed,
        24);

    //==================================================
    // Editor Form Panel
    //==================================================
    auto editorPanel = std::make_shared<Panel>();
    editorPanel->setLayout(std::make_unique<VerticalLayout>());
    editorPanel->setStyle(PanelStyle::Card); // Maintains uniform background formatting
    editorPanel->setBorderVisible(false);
    editorPanel->setSpacing(Metrics::SpaceS);

    rightContainer->addLayoutElement(
        editorPanel,
        SizePolicy::Fill);

    // Form Field: Company
    auto companyLabel = std::make_shared<Label>();
    companyLabel->setText("Company");
    companyLabel->setStyle(LabelStyle::Small);
    companyLabel->setTextTheme(TextTheme::DarkSecondary);
    editorPanel->addLayoutElement(companyLabel, SizePolicy::Fixed, Metrics::SmallRow);

    company = std::make_shared<TextBox>();
    editorPanel->addLayoutElement(company, SizePolicy::Fixed, Metrics::ControlHeight);

    // Form Field: Contact Person
    auto contactLabel = std::make_shared<Label>();
    contactLabel->setText("Contact Person");
    contactLabel->setStyle(LabelStyle::Small);
    contactLabel->setTextTheme(TextTheme::DarkSecondary);
    editorPanel->addLayoutElement(contactLabel, SizePolicy::Fixed, Metrics::SmallRow);

    contact = std::make_shared<TextBox>();
    editorPanel->addLayoutElement(contact, SizePolicy::Fixed, Metrics::ControlHeight);

    // Form Field: Phone
    auto phoneLabel = std::make_shared<Label>();
    phoneLabel->setText("Phone");
    phoneLabel->setStyle(LabelStyle::Small);
    phoneLabel->setTextTheme(TextTheme::DarkSecondary);
    editorPanel->addLayoutElement(phoneLabel, SizePolicy::Fixed, Metrics::SmallRow);

    phone = std::make_shared<TextBox>();
    editorPanel->addLayoutElement(phone, SizePolicy::Fixed, Metrics::ControlHeight);

    // Form Field: Email
    auto emailLabel = std::make_shared<Label>();
    emailLabel->setText("Email");
    emailLabel->setStyle(LabelStyle::Small);
    emailLabel->setTextTheme(TextTheme::DarkSecondary);
    editorPanel->addLayoutElement(emailLabel, SizePolicy::Fixed, Metrics::SmallRow);

    email = std::make_shared<TextBox>();
    editorPanel->addLayoutElement(email, SizePolicy::Fixed, Metrics::ControlHeight);
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
