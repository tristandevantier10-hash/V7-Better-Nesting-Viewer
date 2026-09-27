#include "ResultsScreen.h"

#include "VerticalLayout.h"
#include "HorizontalLayout.h"
#include "NavigationPanel.h"
#include "Metrics.h"
#include "Renderer.h"
#include "CardPanel.h"
#include "TextRenderer.h"
#include <algorithm>
#include <string>
#include <iostream>

ResultsScreen::ResultsScreen()
{
    // ==================================================
    // ROOT
    // ==================================================

    setLayout(
        std::make_unique<VerticalLayout>());

    contentPanel =
        std::make_shared<Panel>();

    contentPanel->setLayout(
        std::make_unique<HorizontalLayout>());

    addLayoutElement(
        contentPanel,
        SizePolicy::Fill);

    // ==================================================
    // SIDEBAR
    // ==================================================

    buildSidebar();

    contentPanel->addLayoutElement(
        sidebar,
        SizePolicy::Fixed,
        Metrics::SidebarWidth);

    // ==================================================
    // MAIN CONTAINER
    // ==================================================

    mainContainer =
        std::make_shared<Panel>();

    mainContainer->setLayout(
        std::make_unique<VerticalLayout>());

    mainContainer->setBorderVisible(false);
    mainContainer->setPadding(34);
    mainContainer->setSpacing(18);

    contentPanel->addLayoutElement(
        mainContainer,
        SizePolicy::Fill);

    // ==================================================
    // HEADER
    // ==================================================

    buildHeader();

    // ==================================================
    // KPI ROW
    // ==================================================

    buildKpiRow();

    // ==================================================
    // CONTENT
    // ==================================================

    buildContentArea();

    // ==================================================
    // BOTTOM ACTIONS
    // ==================================================

    buildBottomActions();
}


// ======================================================
// CARD FACTORY
// ======================================================

std::shared_ptr<Panel> ResultsScreen::createCard()
{
    auto card = std::make_shared<Panel>();

    card->setStyle(PanelStyle::Card);
    card->setBorderVisible(true);
    card->setPadding(18);

    card->setLayout(
        std::make_unique<VerticalLayout>());

    return card;
}

// ======================================================
// SIDEBAR
// ======================================================

void ResultsScreen::buildSidebar()
{
    sidebar =
        std::make_shared<NavigationPanel>();

    sidebar->setLayout(
        std::make_unique<VerticalLayout>());

    sidebar->setSidebarStyle(true);

    sidebar->setPadding(
        Metrics::SidebarPadding);

    sidebar->addItem(
        "RESULTS",
        []() {});

    sidebar->addItem(
        "OVERVIEW",
        []() {});

    sidebar->addItem(
        "NESTING SHEETS",
        []() {});

    sidebar->addItem(
        "COST BREAKDOWN",
        []() {});

    // --------------------------------------------------
    // Spacer
    // --------------------------------------------------

    struct InvisibleSpacer : public Panel
    {
        void render(Renderer& renderer) override
        {
            performLayout();
            renderChildren(renderer);
        }
    };

    auto spacer =
        std::make_shared<InvisibleSpacer>();

    spacer->setBorderVisible(false);

    sidebar->addLayoutElement(
        spacer,
        SizePolicy::Fill);

    // --------------------------------------------------
    // New Job
    // --------------------------------------------------

    sidebar->addItem(
        "NEW JOB",
        [this]()
        {
            if (newJobCallback)
                newJobCallback();
        });

    // --------------------------------------------------
    // Back
    // --------------------------------------------------

    sidebar->addItem(
        "BACK TO MENU",
        [this]()
        {
            if (backCallback)
                backCallback();
        });
}

// ======================================================
// HEADER
// ======================================================

void ResultsScreen::buildHeader()
{
    auto header =
        std::make_shared<Panel>();

    header->setLayout(
        std::make_unique<HorizontalLayout>());

    header->setBorderVisible(false);

    mainContainer->addLayoutElement(
        header,
        SizePolicy::Fixed,
        130);

    // --------------------------------------------------
    // Left
    // --------------------------------------------------

    auto titlePanel =
        std::make_shared<Panel>();

    titlePanel->setLayout(
        std::make_unique<VerticalLayout>());

    titlePanel->setBorderVisible(false);
    titlePanel->setSpacing(5);

    header->addLayoutElement(
        titlePanel,
        SizePolicy::Fill);

    pageTitle =
        std::make_shared<Label>();

    pageTitle->setText(
        "Results");

    pageTitle->setStyle(
        LabelStyle::Heading);

    titlePanel->addLayoutElement(
        pageTitle,
        SizePolicy::Fixed,
        42);

    pageSubtitle =
        std::make_shared<Label>();

    pageSubtitle->setText(
        "Job costing summary and details");

    pageSubtitle->setStyle(
        LabelStyle::Small);

    pageSubtitle->setTextTheme(
        TextTheme::DarkSecondary);

    titlePanel->addLayoutElement(
        pageSubtitle,
        SizePolicy::Fixed,
        26);

    // --------------------------------------------------
    // Customer Card
    // --------------------------------------------------

    customerCard =
        createCard();

    customerCard->setPadding(14);

    header->addLayoutElement(
        customerCard,
        SizePolicy::Fixed,
        330);

    auto customerLayout =
        std::make_shared<Panel>();

    customerLayout->setLayout(
        std::make_unique<VerticalLayout>());

    customerLayout->setBorderVisible(false);

    customerCard->addLayoutElement(
        customerLayout,
        SizePolicy::Fill);

    auto customerTitle =
        std::make_shared<Label>();

    customerTitle->setText(
        "Selected Customer");

    customerTitle->setStyle(
        LabelStyle::Small);

    customerLayout->addLayoutElement(
        customerTitle,
        SizePolicy::Fixed,
        22);

    auto customerName =
        std::make_shared<Label>();

    customerName->setText(
        "Customer");

    customerName->setStyle(
        LabelStyle::Heading);

    customerLayout->addLayoutElement(
        customerName,
        SizePolicy::Fixed,
        30);

    auto customerContact =
        std::make_shared<Label>();

    customerContact->setText(
        "");

    customerLayout->addLayoutElement(
        customerContact,
        SizePolicy::Fixed,
        22);

    auto customerPhone =
        std::make_shared<Label>();

    customerPhone->setText(
        "");

    customerLayout->addLayoutElement(
        customerPhone,
        SizePolicy::Fixed,
        22);
}


// ======================================================
// KPI ROW
// ======================================================

void ResultsScreen::buildKpiRow()
{
    auto row =
        std::make_shared<Panel>();

    row->setLayout(
        std::make_unique<HorizontalLayout>());

    row->setBorderVisible(false);
    row->setSpacing(14);

    mainContainer->addLayoutElement(
        row,
        SizePolicy::Fixed,
        110);

    totalPriceCard =
        createKpiCard(
            "Total Price",
            "R 0.00",
            "Including VAT",
            &totalPriceValueLabel);

    totalCostCard =
        createKpiCard(
            "Total Cost",
            "R 0.00",
            "Excluding VAT",
            &totalCostValueLabel);

    grossProfitCard =
        createKpiCard(
            "Gross Profit",
            "R 0.00",
            "Margin",
            &grossProfitValueLabel);

    itemsCard =
        createKpiCard(
            "Items",
            "0",
            "Job Items",
            &itemsValueLabel);

    sheetsCard =
        createKpiCard(
            "Sheets Used",
            "0",
            "Nesting Sheets",
            &sheetsValueLabel);

    row->addLayoutElement(
        totalPriceCard,
        SizePolicy::Fill);

    row->addLayoutElement(
        totalCostCard,
        SizePolicy::Fill);

    row->addLayoutElement(
        grossProfitCard,
        SizePolicy::Fill);

    row->addLayoutElement(
        itemsCard,
        SizePolicy::Fill);

    row->addLayoutElement(
        sheetsCard,
        SizePolicy::Fill);
}


// ======================================================
// KPI CARD
// ======================================================

std::shared_ptr<Panel>
ResultsScreen::createKpiCard(
    const std::string& title,
    const std::string& value,
    const std::string& subtitle,
    std::shared_ptr<Label>* valueLabelOut)
{
    auto card =
        createCard();

    card->setPadding(14);

    auto layout =
        std::make_shared<Panel>();

    layout->setLayout(
        std::make_unique<VerticalLayout>());

    layout->setBorderVisible(false);
    layout->setSpacing(2);

    card->addLayoutElement(
        layout,
        SizePolicy::Fill);

    auto titleLabel =
        std::make_shared<Label>();

    titleLabel->setText(title);

    titleLabel->setStyle(
        LabelStyle::Small);

    layout->addLayoutElement(
        titleLabel,
        SizePolicy::Fixed,
        22);

    auto valueLabel =
        std::make_shared<Label>();

    valueLabel->setText(value);

    valueLabel->setStyle(
        LabelStyle::Heading);

    if (valueLabelOut)
    {
        *valueLabelOut = valueLabel;
    }

    layout->addLayoutElement(
        valueLabel,
        SizePolicy::Fixed,
        36);

    auto subtitleLabel =
        std::make_shared<Label>();

    subtitleLabel->setText(subtitle);

    subtitleLabel->setStyle(
        LabelStyle::Small);

    subtitleLabel->setTextTheme(
        TextTheme::DarkSecondary);

    layout->addLayoutElement(
        subtitleLabel,
        SizePolicy::Fixed,
        22);

    return card;
}

// ======================================================
// CONTENT AREA
// ======================================================

void ResultsScreen::buildContentArea()
{
    auto content =
        std::make_shared<Panel>();

    content->setLayout(
        std::make_unique<HorizontalLayout>());

    content->setBorderVisible(false);

    content->setSpacing(18);

    mainContainer->addLayoutElement(
        content,
        SizePolicy::Fill);

    // ==================================================
    // LEFT COLUMN
    // ==================================================

    leftColumn =
        std::make_shared<Panel>();

    leftColumn->setLayout(
        std::make_unique<VerticalLayout>());

    leftColumn->setBorderVisible(false);
    leftColumn->setSpacing(16);

    content->addLayoutElement(
        leftColumn,
        SizePolicy::Fill);

    // --------------------------------------------------
    // Cost Breakdown
    // --------------------------------------------------

    costBreakdownCard =
        createCard();

    auto costLayout =
        std::make_shared<Panel>();

    costLayout->setLayout(
        std::make_unique<VerticalLayout>());

    costLayout->setBorderVisible(false);
    costLayout->setSpacing(10);

    costBreakdownCard->addLayoutElement(
        costLayout,
        SizePolicy::Fill);

    auto costTitle =
        std::make_shared<Label>();

    costTitle->setText(
        "Cost Breakdown");

    costTitle->setStyle(
        LabelStyle::Small);

    costLayout->addLayoutElement(
        costTitle,
        SizePolicy::Fixed,
        30);

    costBreakdownGrid =
        std::make_shared<DataGrid>();

    costBreakdownGrid->addColumn(
        "Description",
        180);

    costBreakdownGrid->addColumn(
        "Cost",
        110);

    costBreakdownGrid->addColumn(
        "Markup",
        100);

    costBreakdownGrid->addColumn(
        "Price",
        130);

    costLayout->addLayoutElement(
        costBreakdownGrid,
        SizePolicy::Fill);

    leftColumn->addLayoutElement(
        costBreakdownCard,
        SizePolicy::Fill);

    // --------------------------------------------------
    // Job Items
    // --------------------------------------------------

    jobItemsCard =
        createCard();

    auto jobLayout =
        std::make_shared<Panel>();

    jobLayout->setLayout(
        std::make_unique<VerticalLayout>());

    jobLayout->setBorderVisible(false);
    jobLayout->setSpacing(10);

    jobItemsCard->addLayoutElement(
        jobLayout,
        SizePolicy::Fill);

    auto jobTitle =
        std::make_shared<Label>();

    jobTitle->setText(
        "Job Items");

    jobTitle->setStyle(
        LabelStyle::Small);

    jobLayout->addLayoutElement(
        jobTitle,
        SizePolicy::Fixed,
        30);

    jobItemsGrid =
        std::make_shared<DataGrid>();

    jobItemsGrid->addColumn(
        "Material",
        110);

    jobItemsGrid->addColumn(
        "Variant",
        160);

    jobItemsGrid->addColumn(
        "Size (mm)",
        130);

    jobItemsGrid->addColumn(
        "Qty",
        55);

    jobItemsGrid->addColumn(
        "Sheets",
        65);

    jobLayout->addLayoutElement(
        jobItemsGrid,
        SizePolicy::Fill);

    leftColumn->addLayoutElement(
        jobItemsCard,
        SizePolicy::Fill);

    // ==================================================
    // RIGHT COLUMN
    // ==================================================

    rightColumn =
        std::make_shared<Panel>();

    rightColumn->setLayout(
        std::make_unique<VerticalLayout>());

    rightColumn->setBorderVisible(false);
    rightColumn->setSpacing(16);

    content->addLayoutElement(
        rightColumn,
        SizePolicy::Fixed,
        330);

    // --------------------------------------------------
    // Job Summary
    // --------------------------------------------------

    jobSummaryCard =
        createCard();

    auto summaryLayout =
        std::make_shared<Panel>();

    summaryLayout->setLayout(
        std::make_unique<VerticalLayout>());

    summaryLayout->setBorderVisible(false);

    jobSummaryCard->addLayoutElement(
        summaryLayout,
        SizePolicy::Fill);

    auto summaryTitle =
        std::make_shared<Label>();

    summaryTitle->setText(
        "Job Summary");

    summaryTitle->setStyle(
        LabelStyle::Small);

    summaryLayout->addLayoutElement(
        summaryTitle,
        SizePolicy::Fixed,
        30);

    rightColumn->addLayoutElement(
        jobSummaryCard,
        SizePolicy::Fixed,
        210);

    // --------------------------------------------------
    // Nesting Preview
    // --------------------------------------------------

    nestingCard =
        createCard();

    auto nestingLayout =
        std::make_shared<Panel>();

    nestingLayout->setLayout(
        std::make_unique<VerticalLayout>());

    nestingLayout->setBorderVisible(false);

    nestingCard->addLayoutElement(
        nestingLayout,
        SizePolicy::Fill);

    auto nestingTitle =
        std::make_shared<Label>();

    nestingTitle->setText(
        "Nesting Sheets Preview");

    nestingTitle->setStyle(
        LabelStyle::Small);
            
    nestingLayout->addLayoutElement(
        nestingTitle,
        SizePolicy::Fixed,
        30);

    previewPanel =
        std::make_shared<SheetPreviewPanel>();

    nestingLayout->addLayoutElement(
        previewPanel,
        SizePolicy::Fill);

    rightColumn->addLayoutElement(
        nestingCard,
        SizePolicy::Fill);
}

// ======================================================
// BOTTOM ACTIONS
// ======================================================

void ResultsScreen::buildBottomActions()
{
    auto row =
        std::make_shared<Panel>();

    row->setLayout(
        std::make_unique<HorizontalLayout>());

    row->setBorderVisible(false);
    row->setSpacing(10);

    mainContainer->addLayoutElement(
        row,
        SizePolicy::Fixed,
        54);

    // --------------------------------------------------
    // New Job
    // --------------------------------------------------

    newJobButton =
        std::make_shared<Button>();

    newJobButton->setText(
        "New Job");

    newJobButton->setOnClick(
        [this]()
        {
            if (newJobCallback)
                newJobCallback();
        });

    row->addLayoutElement(
        newJobButton,
        SizePolicy::Fill);

    // --------------------------------------------------
    // Print
    // --------------------------------------------------

    printButton =
        std::make_shared<Button>();

    printButton->setText(
        "Print Quote");

    printButton->setOnClick(
        [this]()
        {
            if (printCallback)
                printCallback();
        });

    row->addLayoutElement(
        printButton,
        SizePolicy::Fill);

    // --------------------------------------------------
    // Export
    // --------------------------------------------------

    exportButton =
        std::make_shared<Button>();

    exportButton->setText(
        "Export PDF");

    exportButton->setOnClick(
        [this]()
        {
            if (exportCallback)
                exportCallback();
        });

    row->addLayoutElement(
        exportButton,
        SizePolicy::Fill);
}

// ======================================================
// VIEW MODEL
// ======================================================

void ResultsScreen::setViewModel(
    const ResultsViewModel& vm)
{
    viewModel = vm;

    refreshView();
}

// ======================================================
// REFRESH
// ======================================================

void ResultsScreen::refreshView()
{
    const auto& invoice =
        viewModel.invoice;

    if (totalPriceValueLabel)
    {
        totalPriceValueLabel->setText(
            invoice.sellPrice);
    }

    if (totalCostValueLabel)
    {
        totalCostValueLabel->setText(
            invoice.subtotal);
    }

    double totalCost = 0.0;
    double totalPrice = 0.0;

    try
    {
        totalCost =
            std::stod(invoice.subtotal);

        totalPrice =
            std::stod(invoice.sellPrice);
    }
    catch (...)
    {
    }

    double profit =
        totalPrice - totalCost;

    std::string profitText =
        "R " + std::to_string(profit);

    if (grossProfitValueLabel)
    {
        grossProfitValueLabel->setText(
            profitText);
    }

    if (itemsValueLabel)
    {
        itemsValueLabel->setText(
            std::to_string(invoice.jobs.size()));
    }

    if (sheetsValueLabel)
    {
        sheetsValueLabel->setText(
            std::to_string(
                viewModel.invoice.jobs.size()));
    }

    refreshCostBreakdown();
    refreshJobItems();
}


// ======================================================
// COST BREAKDOWN
// ======================================================

void ResultsScreen::refreshCostBreakdown()
{
    if (!costBreakdownGrid)
        return;

    costBreakdownGrid->clear();

    costBreakdownGrid->addRow(
        {
            "Materials",
            viewModel.invoice.materialCost,
            viewModel.invoice.markup,
            viewModel.invoice.sellPrice
        });
}


// ======================================================
// JOB ITEMS
// ======================================================

void ResultsScreen::refreshJobItems()
{
    if (!jobItemsGrid)
        return;

    jobItemsGrid->clear();

    for (const auto& job :
        viewModel.invoice.jobs)
    {
        std::string size;

        if (job.isRoll)
        {
            size =
                job.rollWidth +
                " x " +
                job.lengthUsed;
        }
        else
        {
            size =
                job.sheetSize;
        }

        jobItemsGrid->addRow(
            {
                job.material,
                job.variant,
                size,
                job.quantity,
                job.isRoll
                    ? job.sheetsUsed
                    : job.sheetsUsed
            });
    }
}


// ======================================================
// SHEETS
// ======================================================

void ResultsScreen::setSheets(
    const std::vector<Sheet>& sheets)
{
    if (previewPanel)
        previewPanel->setSheets(sheets);
}


// ======================================================
// UPDATE
// ======================================================

void ResultsScreen::update(
    const SDL_Event& e)
{
    Panel::update(e);
}

// ======================================================
// RENDER
// ======================================================

void ResultsScreen::render(
    Renderer& renderer)
{
    performLayout();

    std::cout
        << "\n========== RESULTS LAYOUT ==========\n";

    std::cout
        << "SCREEN       : "
        << getX() << ", "
        << getY() << " "
        << getWidth() << "x"
        << getHeight()
        << "\n";

    if (contentPanel)
    {
        std::cout
            << "CONTENT      : "
            << contentPanel->getX() << ", "
            << contentPanel->getY() << " "
            << contentPanel->getWidth() << "x"
            << contentPanel->getHeight()
            << "\n";
    }

    if (mainContainer)
    {
        std::cout
            << "MAIN         : "
            << mainContainer->getX() << ", "
            << mainContainer->getY() << " "
            << mainContainer->getWidth() << "x"
            << mainContainer->getHeight()
            << "\n";
    }

    if (leftColumn)
    {
        std::cout
            << "LEFT         : "
            << leftColumn->getX() << ", "
            << leftColumn->getY() << " "
            << leftColumn->getWidth() << "x"
            << leftColumn->getHeight()
            << "\n";
    }

    if (rightColumn)
    {
        std::cout
            << "RIGHT        : "
            << rightColumn->getX() << ", "
            << rightColumn->getY() << " "
            << rightColumn->getWidth() << "x"
            << rightColumn->getHeight()
            << "\n";
    }

    if (costBreakdownCard)
    {
        std::cout
            << "COST CARD    : "
            << costBreakdownCard->getX() << ", "
            << costBreakdownCard->getY() << " "
            << costBreakdownCard->getWidth() << "x"
            << costBreakdownCard->getHeight()
            << "\n";
    }

    if (costBreakdownGrid)
    {
        std::cout
            << "COST GRID    : "
            << costBreakdownGrid->getX() << ", "
            << costBreakdownGrid->getY() << " "
            << costBreakdownGrid->getWidth() << "x"
            << costBreakdownGrid->getHeight()
            << "\n";
    }

    if (jobItemsGrid)
    {
        std::cout
            << "JOB GRID     : "
            << jobItemsGrid->getX() << ", "
            << jobItemsGrid->getY() << " "
            << jobItemsGrid->getWidth() << "x"
            << jobItemsGrid->getHeight()
            << "\n";
    }

    std::cout
        << "====================================\n";

    renderBackground(renderer);

    renderChildren(renderer);
}

// ======================================================
// CALLBACKS
// ======================================================

void ResultsScreen::setNewJobCallback(
    std::function<void()> callback)
{
    newJobCallback = callback;
}

void ResultsScreen::setBackCallback(
    std::function<void()> callback)
{
    backCallback = callback;
}

void ResultsScreen::setPrintCallback(
    std::function<void()> callback)
{
    printCallback = callback;
}

void ResultsScreen::setExportCallback(
    std::function<void()> callback)
{
    exportCallback = callback;
}