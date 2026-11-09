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

CustomerEditorScreen::CustomerEditorScreen(std::shared_ptr<NavigationPanel> sharedNavigation)
{
    //==================================================
    // Root Layout
    //==================================================
    setPadding(0);
    setSpacing(0);

    auto contentPanel = std::make_shared<Panel>();
    contentPanel->setLayout(std::make_unique<VerticalLayout>());
    contentPanel->setPadding(0);
    contentPanel->setSpacing(0);

    //==================================================
    // Sidebar
    //==================================================
    auto navigation = std::move(sharedNavigation);

    //==================================================
    // Right Side Container
    //==================================================
    auto rightContainer = std::make_shared<Panel>();
    rightContainer->setLayout(std::make_unique<VerticalLayout>());
    rightContainer->setBorderVisible(false);
    rightContainer->setStyle(PanelStyle::Workspace);
    rightContainer->setPadding(Metrics::WorkspaceGap);
    rightContainer->setSpacing(Metrics::WorkspaceGap);

    //==================================================
    // Page Header (Integrated into Main Area)
    //==================================================
    auto pageHeader = std::make_shared<DashboardBannerPanel>();
    pageHeader->setLayout(std::make_unique<HorizontalLayout>());
    configurePageHeader(pageHeader);
    auto workspaceBody = addWorkspaceLayout(*this, pageHeader);
    workspaceBody->addLayoutElement(navigation, SizePolicy::Fixed, Metrics::SidebarWidth);
    workspaceBody->addLayoutElement(contentPanel, SizePolicy::Fill);

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

    saveButton = std::make_shared<Button>();
    saveButton->setText("Save");
    saveButton->setOnClick([this]()
    {
        if (saveCallback)
            saveCallback();
    });
    addPageHeaderActions(pageHeader, saveButton, cancelButton);

    contentPanel->addLayoutElement(rightContainer, SizePolicy::Fill);

    //==================================================
    // Editor Form Panel
    //==================================================
    auto editorPanel = std::make_shared<Panel>();
    editorPanel->setLayout(std::make_unique<VerticalLayout>());
    editorPanel->setStyle(PanelStyle::Card);
    editorPanel->setPadding(Metrics::CardPadding);
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
    company->setCompactText(true);
    editorPanel->addLayoutElement(company, SizePolicy::Fixed, Metrics::CompactControlHeight);

    // Form Field: Contact Person
    auto contactLabel = std::make_shared<Label>();
    contactLabel->setText("Contact Person");
    contactLabel->setStyle(LabelStyle::Small);
    contactLabel->setTextTheme(TextTheme::DarkSecondary);
    editorPanel->addLayoutElement(contactLabel, SizePolicy::Fixed, Metrics::SmallRow);

    contact = std::make_shared<TextBox>();
    contact->setCompactText(true);
    editorPanel->addLayoutElement(contact, SizePolicy::Fixed, Metrics::CompactControlHeight);

    // Form Field: Phone
    auto phoneLabel = std::make_shared<Label>();
    phoneLabel->setText("Phone");
    phoneLabel->setStyle(LabelStyle::Small);
    phoneLabel->setTextTheme(TextTheme::DarkSecondary);
    editorPanel->addLayoutElement(phoneLabel, SizePolicy::Fixed, Metrics::SmallRow);

    phone = std::make_shared<TextBox>();
    phone->setCompactText(true);
    editorPanel->addLayoutElement(phone, SizePolicy::Fixed, Metrics::CompactControlHeight);

    // Form Field: Email
    auto emailLabel = std::make_shared<Label>();
    emailLabel->setText("Email");
    emailLabel->setStyle(LabelStyle::Small);
    emailLabel->setTextTheme(TextTheme::DarkSecondary);
    editorPanel->addLayoutElement(emailLabel, SizePolicy::Fixed, Metrics::SmallRow);

    email = std::make_shared<TextBox>();
    email->setCompactText(true);
    editorPanel->addLayoutElement(email, SizePolicy::Fixed, Metrics::CompactControlHeight);
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
