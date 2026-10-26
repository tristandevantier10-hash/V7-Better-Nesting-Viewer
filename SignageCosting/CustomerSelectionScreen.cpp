#include "CustomerSelectionScreen.h"
#include "VerticalLayout.h"
#include "ListView.h"
#include "CustomerDatabase.h"
#include "HorizontalLayout.h"
#include "NavigationPanel.h"
#include "Metrics.h"
#include "CardPanel.h"
#include "Button.h"
#include "PageHeader.h"

CustomerSelectionScreen::CustomerSelectionScreen()
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

    auto sidebar = std::make_shared<NavigationPanel>();
    sidebar->setLayout(std::make_unique<VerticalLayout>());
    sidebar->setSidebarStyle(true);
    sidebar->setPadding(Metrics::SidebarPadding);

    contentPanel->addLayoutElement(
        sidebar,
        SizePolicy::Fixed,
        Metrics::SidebarWidth);

    sidebar->addHeading("CUSTOMERS");

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

    sidebar->addLayoutElement(spacer, SizePolicy::Fill);

    sidebar->addItem(
        "BACK",
        [this]()
        {
            if (backCallback)
                backCallback();
        });

    //==================================================
    // Right Side
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
    // Page Header
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
    addPageHeaderLogo(pageHeader, "Assets/Images/E&G Signs cc.png");

    pageHeader->addLayoutElement(
        titlePanel,
        SizePolicy::Fill);

    auto title = std::make_shared<Label>();
    title->setText("Customers");
    title->setStyle(LabelStyle::Heading);
    configurePageHeaderTitleLabel(title);

    titlePanel->addLayoutElement(
        title,
        SizePolicy::Fixed,
        36);

    auto subtitle = std::make_shared<Label>();
    subtitle->setText("Manage your customer database");
    subtitle->setStyle(LabelStyle::Small);
    configurePageHeaderSubtitleLabel(subtitle);

    titlePanel->addLayoutElement(
        subtitle,
        SizePolicy::Fixed,
        24);

    auto newCustomerButton = std::make_shared<Button>();
    newCustomerButton->setText("+  New Customer");
    newCustomerButton->setOnClick(
        [this]()
        {
            if (newCustomerCallback)
                newCustomerCallback();
        });

    auto newCustomerButtonContainer =
        std::make_shared<Panel>();

    auto newCustomerButtonLayout = std::make_unique<VerticalLayout>();
    newCustomerButtonLayout->setCenterContent(true);
    newCustomerButtonContainer->setLayout(std::move(newCustomerButtonLayout));

    newCustomerButtonContainer->setBorderVisible(false);
    newCustomerButtonContainer->setBackgroundColour(
        { 255, 255, 255, 255 });
    newCustomerButtonContainer->setPadding(0);

    newCustomerButtonContainer->addLayoutElement(
        newCustomerButton,
        SizePolicy::Fixed,
        40);

    pageHeader->addLayoutElement(
        newCustomerButtonContainer,
        SizePolicy::Fixed,
        180);

    //==================================================
    // Workspace
    //==================================================

    auto workspace = std::make_shared<Panel>();
    workspace->setLayout(std::make_unique<VerticalLayout>());
    workspace->setStyle(PanelStyle::Card);
    workspace->setBorderVisible(false);
    workspace->setPadding(0);
    workspace->setSpacing(Metrics::SpaceL);

    rightContainer->addLayoutElement(
        workspace,
        SizePolicy::Fill);

    //==================================================
    // Toolbar
    //==================================================

    auto toolbar = std::make_shared<Panel>();
    toolbar->setLayout(std::make_unique<VerticalLayout>());
    toolbar->setStyle(PanelStyle::Card);
    toolbar->setBorderVisible(false);
    toolbar->setPadding(0);
    toolbar->setSpacing(Metrics::SpaceM);

    workspace->addLayoutElement(
        toolbar,
        SizePolicy::Fixed,
        100);

    //==================================================
    // Account Selector
    //==================================================

    accountSelector = std::make_shared<SegmentedControl>();

    accountSelector->addSegment("Cash Customers");
    accountSelector->addSegment("Credit Customers");

    accountSelector->setSelectionChangedCallback(
        [this](int index)
        {
            currentAccountType =
                (index == 0)
                ? AccountType::Cash
                : AccountType::Credit;

            refreshCustomers();
        });

    toolbar->addLayoutElement(
        accountSelector,
        SizePolicy::Fixed,
        46);

    //==================================================
    // Search
    //==================================================

    auto searchRow = std::make_shared<Panel>();

    searchRow->setLayout(
        std::make_unique<HorizontalLayout>());

    searchRow->setBorderVisible(false);
    searchRow->setSpacing(Metrics::SpaceM);
    searchRow->setBackgroundColour(
        { 255,255,255,255 });

    toolbar->addLayoutElement(
        searchRow,
        SizePolicy::Fixed,
        42);

    searchBox = std::make_shared<TextBox>();

    searchBox->setPlaceholder(
        "Search customers...");

    searchRow->addLayoutElement(
        searchBox,
        SizePolicy::Fill);

    //==================================================
    // Customer Grid
    //==================================================

    customerGrid = std::make_shared<DataGrid>();

    customerGrid->addColumn("Company", 400);
    customerGrid->addColumn("Contact", 250);
    customerGrid->addColumn("Phone", 200);
    customerGrid->addColumn("Type", 150);

    workspace->addLayoutElement(
        customerGrid,
        SizePolicy::Fill);

    //==================================================
    // Customer Actions
    //==================================================

    auto editButton = std::make_shared<Button>();
    editButton->setText("Edit");

    editButton->setOnClick(
        [this]()
        {
            int index = customerGrid->getSelectedRow();

            if (index >= 0)
            {
                if (editCustomerCallback)
                    editCustomerCallback(index);
            }
        });

    workspace->addLayoutElement(
        editButton,
        SizePolicy::Fixed,
        42);

    auto deleteButton = std::make_shared<Button>();
    deleteButton->setText("Delete");

    deleteButton->setOnClick(
        [this]()
        {
            int index = customerGrid->getSelectedRow();

            if (index >= 0)
            {
                if (deleteCustomerCallback)
                    deleteCustomerCallback(index);
            }
        });

    workspace->addLayoutElement(
        deleteButton,
        SizePolicy::Fixed,
        42);

    selectButton = std::make_shared<Button>();
    selectButton->setText("Select");

    selectButton->setOnClick(
        [this]()
        {
            int index = customerGrid->getSelectedRow();

            if (index >= 0)
            {
                if (selectCustomerCallback)
                    selectCustomerCallback(index);
            }
        });

    workspace->addLayoutElement(
        selectButton,
        SizePolicy::Fixed,
        42);

    refreshCustomers();
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
    customerGrid->clear();

    const auto& allCustomers = CustomerDatabase::getAll();

    for (int i = 0;
        i < static_cast<int>(allCustomers.size());
        ++i)
    {
        const auto& customer = allCustomers[i];

        // Only show the selected account type
        if (customer.accountType != currentAccountType)
            continue;

        customerGrid->addRow(
            {
                customer.company,
                customer.contact,
                customer.phone,
                customer.accountType == AccountType::Cash
                    ? "Cash"
                    : "Credit"
            },
            i);   // <-- actual database index
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

void CustomerSelectionScreen::setBackCallback(std::function<void()> callback)
{
    backCallback = callback;
}

void CustomerSelectionScreen::setSelectionMode(bool enabled)
{
    selectButton->setVisible(enabled);
}
