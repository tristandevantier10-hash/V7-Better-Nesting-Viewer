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
#include <utility>

CustomerEditorScreen::CustomerEditorScreen()
{
    //==================================================
    // Root Layout
    //==================================================
    setLayout(std::make_unique<HorizontalLayout>());
    setPadding(0);
    setSpacing(0);

    auto contentPanel = std::make_shared<Panel>();
    contentPanel->setLayout(std::make_unique<VerticalLayout>());
    contentPanel->setPadding(0);
    contentPanel->setSpacing(0);

    //==================================================
    // Sidebar
    //==================================================
    auto navigation = createPrimaryNavigationPanel(
        PrimaryNavigationRoute::CustomerDatabase,
        [this](PrimaryNavigationRoute route)
        {
            if (navigationCallback)
                navigationCallback(route);
        });
    addLayoutElement(navigation, SizePolicy::Fixed, Metrics::SidebarWidth);
    addLayoutElement(contentPanel, SizePolicy::Fill);

    //==================================================
    // Right Side Container
    //==================================================
    auto rightContainer = std::make_shared<Panel>();
    rightContainer->setLayout(std::make_unique<VerticalLayout>());
    rightContainer->setStyle(PanelStyle::Card);
    rightContainer->setBorderVisible(false);
    rightContainer->setPadding(Metrics::SpaceXL);
    rightContainer->setSpacing(Metrics::SpaceL);

    //==================================================
    // Page Header (Integrated into Main Area)
    //==================================================
    auto pageHeader = std::make_shared<DashboardBannerPanel>();
    pageHeader->setLayout(std::make_unique<HorizontalLayout>());
    configurePageHeader(pageHeader);
    contentPanel->addLayoutElement(pageHeader, SizePolicy::Fixed, 132);

    auto titlePanel = std::make_shared<PageHeaderContentPanel>();
    titlePanel->setLayout(std::make_unique<VerticalLayout>());
    configurePageHeaderTitle(titlePanel);
    pageHeader->addLayoutElement(
        titlePanel,
        SizePolicy::Fill);

    auto title = std::make_shared<Label>();
    title->setText("Customer & Invoice");
    title->setStyle(LabelStyle::Heading);
    configurePageHeaderTitleLabel(title);

    titlePanel->addLayoutElement(
        title,
        SizePolicy::Fixed,
        36);

    auto subtitle = std::make_shared<Label>();
    subtitle->setText("Customer Management");
    subtitle->setStyle(LabelStyle::Small);
    configurePageHeaderSubtitleLabel(subtitle);

    titlePanel->addLayoutElement(
        subtitle,
        SizePolicy::Fixed,
        24);

    cancelButton = std::make_shared<Button>();
    cancelButton->setText("Cancel");
    cancelButton->setOnClick([this]()
    {
        if (cancelCallback)
            cancelCallback();
    });
    pageHeader->addLayoutElement(cancelButton, SizePolicy::Fixed, 100);

    saveButton = std::make_shared<Button>();
    saveButton->setText("Save");
    saveButton->setOnClick([this]()
    {
        if (saveCallback)
            saveCallback();
    });
    pageHeader->addLayoutElement(saveButton, SizePolicy::Fixed, 100);

    contentPanel->addLayoutElement(rightContainer, SizePolicy::Fill);

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

void CustomerEditorScreen::setNavigationCallback(
    std::function<void(PrimaryNavigationRoute)> callback)
{
    navigationCallback = std::move(callback);
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
