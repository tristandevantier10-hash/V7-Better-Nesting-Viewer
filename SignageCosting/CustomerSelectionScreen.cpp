#include "CustomerSelectionScreen.h"
#include "VerticalLayout.h"
#include "ListView.h"
#include "CustomerDatabase.h"
#include "HorizontalLayout.h"
#include "NavigationPanel.h"
#include "Metrics.h"
#include "CardPanel.h"
#include "Button.h"

CustomerSelectionScreen::CustomerSelectionScreen()
{
    setLayout(std::make_unique<VerticalLayout>());

    auto contentPanel = std::make_shared<Panel>();
    contentPanel->setLayout(std::make_unique<HorizontalLayout>());

    addLayoutElement(
        contentPanel,
        SizePolicy::Fill);

    auto sidebar = std::make_shared<NavigationPanel>();
    sidebar->setLayout(std::make_unique<VerticalLayout>());
    sidebar->setSidebarStyle(true);
    sidebar->setPadding(Metrics::SidebarPadding); // Added padding to give top/bottom items breathing room

    contentPanel->addLayoutElement(
        sidebar,
        SizePolicy::Fixed,
        Metrics::SidebarWidth);

    // 1. Top item
    sidebar->addItem(
        "CUSTOMERS",
        []() {});

    // 2. Empty spacer panel with its rendering completely stripped out
    struct InvisibleSpacer : public Panel {
        void render(Renderer& renderer) override {
            // Bypass renderBackground() completely!
            performLayout();
            renderChildren(renderer);
        }
    };
    auto sidebarSpacer = std::make_shared<InvisibleSpacer>();
    sidebarSpacer->setBorderVisible(false); // Keeps layout bounds clean

    sidebar->addLayoutElement(sidebarSpacer, SizePolicy::Fill);

    // 3. Bottom item
    sidebar->addItem(
        "BACK",
        [this]()
        {
            if (backCallback)
                backCallback();
        });

    auto rightContainer = std::make_shared<Panel>();
    rightContainer->setLayout(std::make_unique<VerticalLayout>());
    rightContainer->setBorderVisible(false);
    rightContainer->setPadding(25);
    rightContainer->setSpacing(20);

    contentPanel->addLayoutElement(
        rightContainer,
        SizePolicy::Fill);

    auto pageHeader = std::make_shared<Panel>();
    pageHeader->setLayout(std::make_unique<HorizontalLayout>());
    pageHeader->setBorderVisible(false);

    rightContainer->addLayoutElement(
        pageHeader,
        SizePolicy::Fixed,
        70);

    auto titlePanel = std::make_shared<Panel>();
    titlePanel->setLayout(std::make_unique<VerticalLayout>());
    titlePanel->setBorderVisible(false);
    titlePanel->setSpacing(4);

    pageHeader->addLayoutElement(
        titlePanel,
        SizePolicy::Fill);

    auto title = std::make_shared<Label>();
    title->setText("Customers");
    title->setStyle(LabelStyle::Heading);

    titlePanel->addLayoutElement(
        title,
        SizePolicy::Fixed,
        36);

    auto newCustomerButton = std::make_shared<Button>();
    newCustomerButton->setText("+  New Customer");

    newCustomerButton->setOnClick(
        [this]()
        {
            if (newCustomerCallback)
                newCustomerCallback();
        });

    pageHeader->addLayoutElement(
        newCustomerButton,
        SizePolicy::Fixed,
        180);

    auto subtitle = std::make_shared<Label>();
    subtitle->setText("Manage your customer database");
    subtitle->setStyle(LabelStyle::Small);
    subtitle->setTextTheme(TextTheme::DarkSecondary);

    titlePanel->addLayoutElement(
        subtitle,
        SizePolicy::Fixed,
        24);

    auto customerCard = std::make_shared<CardPanel>();

    customerCard->setPadding(20);
    customerCard->setSpacing(18);
    customerCard->setBorderVisible(false);

    rightContainer->addLayoutElement(
        customerCard,
        SizePolicy::Fill);

    accountSelector = std::make_shared<SegmentedControl>();

    accountSelector->addSegment("Cash Customers");
    accountSelector->addSegment("Credit Customers");

    accountSelector->setSelectionChangedCallback(
        [this](int index)
        {
            if (index == 0)
                currentAccountType = AccountType::Cash;
            else
                currentAccountType = AccountType::Credit;

            refreshCustomers();
        });

    customerCard->addLayoutElement(
        accountSelector,
        SizePolicy::Fixed,
        46);

    //==================================================
    // Search Row
    //==================================================

    auto searchRow = std::make_shared<Panel>();
    searchRow->setLayout(std::make_unique<HorizontalLayout>());
    searchRow->setBorderVisible(false);
    searchRow->setSpacing(12);

    customerCard->addLayoutElement(
        searchRow,
        SizePolicy::Fixed,
        42);

    searchBox = std::make_shared<TextBox>();
    searchBox->setPlaceholder("Search customers...");

    searchRow->addLayoutElement(
        searchBox,
        SizePolicy::Fill);

    auto company = std::make_shared<Label>();
    company->setText("Company");
    company->setStyle(LabelStyle::Small);
    company->setTextTheme(TextTheme::DarkSecondary);

    auto contact = std::make_shared<Label>();
    contact->setText("Contact");
    contact->setStyle(LabelStyle::Small);
    contact->setTextTheme(TextTheme::DarkSecondary);

    auto phone = std::make_shared<Label>();
    phone->setText("Phone");
    phone->setStyle(LabelStyle::Small);
    phone->setTextTheme(TextTheme::DarkSecondary);

    auto type = std::make_shared<Label>();
    type->setText("Type");
    type->setStyle(LabelStyle::Small);
    type->setTextTheme(TextTheme::DarkSecondary);

    //==================================================
    // Table Header
    //==================================================

    auto tableHeader = std::make_shared<Panel>();
    tableHeader->setLayout(std::make_unique<HorizontalLayout>());
    tableHeader->setBorderVisible(false);
    tableHeader->setPadding(8);
    tableHeader->setSpacing(10);

    customerCard->addLayoutElement(
        tableHeader,
        SizePolicy::Fixed,
        32);

    tableHeader->addLayoutElement(
        company,
        SizePolicy::Fill);

    tableHeader->addLayoutElement(
        contact,
        SizePolicy::Fixed,
        180);

    tableHeader->addLayoutElement(
        phone,
        SizePolicy::Fixed,
        140);

    tableHeader->addLayoutElement(
        type,
        SizePolicy::Fixed,
        100);

    //==================================================
    // Customer List
    //==================================================

    customerGrid = std::make_shared<DataGrid>();

    customerGrid->addColumn("Company", 350);
    customerGrid->addColumn("Contact", 180);
    customerGrid->addColumn("Phone", 150);
    customerGrid->addColumn("Type", 100);

    customerCard->addLayoutElement(
        customerGrid,
        SizePolicy::Fill);

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

    std::vector<Customer> customers;

    if (currentAccountType == AccountType::Cash)
        customers = CustomerDatabase::getCashCustomers();
    else
        customers = CustomerDatabase::getCreditCustomers();

    for (const auto& customer : customers)
    {
        customerGrid->addRow(
            {
                customer.company,
                customer.contact,
                customer.phone,
                customer.accountType == AccountType::Cash
                    ? "Cash"
                    : "Credit"
            });
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