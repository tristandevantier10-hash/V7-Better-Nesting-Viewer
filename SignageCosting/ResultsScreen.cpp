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
#include "ScrollPanel.h"

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
    // INITIAL PAGE
    // ==================================================

    showOverview();
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

    sidebar->addHeading("RESULTS");

    sidebar->addItem(
        "OVERVIEW",
        [this]()
        {
            showOverview();
        },
        "Assets/Icons/overview.svg");

    sidebar->addItem(
        "NESTING SHEETS",
        [this]()
        {
            showNestingSheets();
        },
        "Assets/Icons/nesting.svg");

    sidebar->addItem(
        "COST BREAKDOWN",
        [this]()
        {
            showCostBreakdown();
        },
        "Assets/Icons/costing.svg");

    sidebar->addItem(
        "QUOTE",
        [this]()
        {
            showQuote();
        },
        "Assets/Icons/quote.svg");

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
        },
        "Assets/Icons/new_job.svg");

    // --------------------------------------------------
    // Back
    // --------------------------------------------------

    sidebar->addItem(
        "BACK TO MENU",
        [this]()
        {
            if (backCallback)
                backCallback();
        },
        "Assets/Icons/back.svg");
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
    // Client Info
    // --------------------------------------------------

    clientInfoCard =
        createCard();

    auto clientInfoLayout =
        std::make_shared<Panel>();

    clientInfoLayout->setLayout(
        std::make_unique<VerticalLayout>());

    clientInfoLayout->setBorderVisible(false);
    clientInfoLayout->setSpacing(10);

    clientInfoCard->addLayoutElement(
        clientInfoLayout,
        SizePolicy::Fill);

    // --------------------------------------------------
    // Title
    // --------------------------------------------------

    auto summaryTitle =
        std::make_shared<Label>();

    summaryTitle->setText(
        "Job Summary");

    summaryTitle->setStyle(
        LabelStyle::Normal);

    clientInfoLayout->addLayoutElement(
        summaryTitle,
        SizePolicy::Fixed,
        20);

    // --------------------------------------------------
    // Job Reference - 50/50 Split Row
    // --------------------------------------------------
    auto row1 = std::make_shared<Panel>();
    row1->setBorderVisible(false);

    row1->setLayout(std::make_unique<HorizontalLayout>());

    jobReferenceLabel = std::make_shared<Label>();
    jobReferenceLabel->setText("Job Reference");
    jobReferenceLabel->setStyle(LabelStyle::Small);

    jobReferenceValueLabel = std::make_shared<Label>();
    jobReferenceValueLabel->setText("REF-001");
    jobReferenceValueLabel->setStyle(LabelStyle::Small);

    row1->addLayoutElement(jobReferenceLabel, SizePolicy::Fill); // Left side
    row1->addLayoutElement(jobReferenceValueLabel, SizePolicy::Fill);   // Right side
    clientInfoLayout->addLayoutElement(row1, SizePolicy::Fixed, 18);

    // --------------------------------------------------
    // Date - 50/50 Split Row
    // --------------------------------------------------
    auto row2 = std::make_shared<Panel>();
    row2->setBorderVisible(false);
    row2->setLayout(std::make_unique<HorizontalLayout>());

    dateSummaryLabel = std::make_shared<Label>();
    dateSummaryLabel->setText("Date");
    dateSummaryLabel->setStyle(LabelStyle::Small);

    dateValueLabel = std::make_shared<Label>();
    dateValueLabel->setText(""); // Dynamically populated by refreshView()
    dateValueLabel->setStyle(LabelStyle::Small);

    row2->addLayoutElement(dateSummaryLabel, SizePolicy::Fill, 50); // Left side
    row2->addLayoutElement(dateValueLabel, SizePolicy::Fill, 50);   // Right side
    clientInfoLayout->addLayoutElement(row2, SizePolicy::Fixed, 18);

    // --------------------------------------------------
    // Payment Plan - 50/50 Split Row
    // --------------------------------------------------
    auto row3 = std::make_shared<Panel>();
    row3->setBorderVisible(false);
    row3->setLayout(std::make_unique<HorizontalLayout>());

    paymentTermsLabel = std::make_shared<Label>();
    paymentTermsLabel->setText("Payment Terms");
    paymentTermsLabel->setStyle(LabelStyle::Small);

    paymentTermsValueLabel = std::make_shared<Label>();
    paymentTermsValueLabel->setText("COD");
    paymentTermsValueLabel->setStyle(LabelStyle::Small);

    row3->addLayoutElement(paymentTermsLabel, SizePolicy::Fill, 50); // Left side
    row3->addLayoutElement(paymentTermsValueLabel, SizePolicy::Fill, 50);   // Right side
    clientInfoLayout->addLayoutElement(row3, SizePolicy::Fixed, 18);

    // --------------------------------------------------
    // Validity Period - 50/50 Split Row
    // --------------------------------------------------
    auto row4 = std::make_shared<Panel>();
    row4->setBorderVisible(false);
    row4->setLayout(std::make_unique<HorizontalLayout>());

    validForLabel = std::make_shared<Label>();
    validForLabel->setText("Valid For");
    validForLabel->setStyle(LabelStyle::Small);

    validForValueLabel = std::make_shared<Label>();
    validForValueLabel->setText("30 Days");
    validForValueLabel->setStyle(LabelStyle::Small);

    row4->addLayoutElement(validForLabel, SizePolicy::Fill, 50); // Left side
    row4->addLayoutElement(validForValueLabel, SizePolicy::Fill, 50);   // Right side
    clientInfoLayout->addLayoutElement(row4, SizePolicy::Fixed, 18);

    // Job summary to change to Client Info block

    rightColumn->addLayoutElement(
        clientInfoCard,
        SizePolicy::Fixed,
        170);

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
        std::string costText = invoice.subtotal;
        std::string priceText = invoice.sellPrice;

        if (!costText.empty() && costText[0] == 'R')
            costText.erase(0, 1);

        if (!priceText.empty() && priceText[0] == 'R')
            priceText.erase(0, 1);

        totalCost =
            std::stod(costText);

        totalPrice =
            std::stod(priceText);
    }
    catch (...)
    {
    }

    double profit =
        totalPrice - totalCost;

    std::string profitText =
        "R " + std::to_string(static_cast<int>(profit));

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
                viewModel.sheetsUsed));
    }

    if (jobReferenceLabel)
    {
        jobReferenceLabel->setText(
            "Job Reference"); // Provide a hardcoded or custom reference instead of a currency cost
    }

    if (dateSummaryLabel)
    {
        dateSummaryLabel->setText("Date"); // Left box gets JUST the title text
    }
    if (dateValueLabel)
    {
        dateValueLabel->setText(viewModel.invoice.invoiceDate); // Right box gets JUST the date numbers
    }

    if (paymentTermsLabel)
    {
        paymentTermsLabel->setText(
            "Payment Terms"); // Changed from productionCost to clean text string
    }

    if (validForLabel)
    {
        validForLabel->setText(
            "Valid For"); // Changed from subtotal cost to clean text string
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
                job.sheetsUsed
            });
    }
}

// ======================================================
// SHEETS
// ======================================================

void ResultsScreen::setSheets(
    const std::vector<Sheet>& sheets)
{
    nestingSheets = sheets;

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

void ResultsScreen::setOverviewCallback(std::function<void()> callback)
{
    overviewCallback = callback;
}

void ResultsScreen::setNestingSheetsCallback(std::function<void()> callback)
{
    nestingSheetsCallback = callback;
}

void ResultsScreen::setCostBreakdownCallback(std::function<void()> callback)
{
    costBreakdownCallback = callback;
}

void ResultsScreen::setQuoteCallback(std::function<void()> callback)
{
    quoteCallback = callback;
}

// ======================================================
// PAGE SWITCHING
// ======================================================

void ResultsScreen::clearMainContainer()
{
    if (mainContainer)
    {
        mainContainer->clearChildren();
    }
}

void ResultsScreen::showOverview()
{
    clearMainContainer();

    buildHeader();
    buildKpiRow();
    buildContentArea();
    buildBottomActions();

    // Refresh the newly created controls
    // using the existing job data.
    refreshView();

    // Restore the existing nesting result.
    if (previewPanel)
    {
        previewPanel->setSheets(nestingSheets);
    }
}

void ResultsScreen::showNestingSheets()
{
    clearMainContainer();

    // --------------------------------------------------
    // Header
    // --------------------------------------------------

    auto header = std::make_shared<Panel>();

    header->setLayout(
        std::make_unique<VerticalLayout>());

    header->setBorderVisible(false);
    header->setSpacing(4);

    mainContainer->addLayoutElement(
        header,
        SizePolicy::Fixed,
        70);

    auto title = std::make_shared<Label>();

    title->setText(
        "Nesting Sheets");

    title->setStyle(
        LabelStyle::Heading);

    header->addLayoutElement(
        title,
        SizePolicy::Fixed,
        38);

    auto subtitle = std::make_shared<Label>();

    subtitle->setText(
        "Material nesting and sheet utilisation");

    subtitle->setStyle(
        LabelStyle::Small);

    subtitle->setTextTheme(
        TextTheme::DarkSecondary);

    header->addLayoutElement(
        subtitle,
        SizePolicy::Fixed,
        24);

    // --------------------------------------------------
    // Nesting Card
    // --------------------------------------------------

    auto nestingCard = createCard();

    mainContainer->addLayoutElement(
        nestingCard,
        SizePolicy::Fill);

    auto nestingLayout = std::make_shared<Panel>();

    nestingLayout->setLayout(
        std::make_unique<VerticalLayout>());

    nestingLayout->setBorderVisible(false);
    nestingLayout->setSpacing(10);

    nestingCard->addLayoutElement(
        nestingLayout,
        SizePolicy::Fill);

    // --------------------------------------------------
    // Card Title
    // --------------------------------------------------

    auto nestingTitle = std::make_shared<Label>();

    nestingTitle->setText(
        "Nesting Preview");

    nestingTitle->setStyle(
        LabelStyle::Normal);

    nestingLayout->addLayoutElement(
        nestingTitle,
        SizePolicy::Fixed,
        30);

    // --------------------------------------------------
    // Preview
    // --------------------------------------------------

    auto fullPreview =
        std::make_shared<SheetPreviewPanel>();

    fullPreview->setSheets(
        nestingSheets);

    nestingLayout->addLayoutElement(
        fullPreview,
        SizePolicy::Fill);
}

void ResultsScreen::showCostBreakdown()
{
    clearMainContainer();

    // --------------------------------------------------
    // Header
    // --------------------------------------------------

    auto header =
        std::make_shared<Panel>();

    header->setLayout(
        std::make_unique<VerticalLayout>());

    header->setBorderVisible(false);
    header->setSpacing(4);

    mainContainer->addLayoutElement(
        header,
        SizePolicy::Fixed,
        70);

    auto title =
        std::make_shared<Label>();

    title->setText(
        "Cost Breakdown");

    title->setStyle(
        LabelStyle::Heading);

    header->addLayoutElement(
        title,
        SizePolicy::Fixed,
        38);

    auto subtitle =
        std::make_shared<Label>();

    subtitle->setText(
        "Detailed breakdown of job costs and selling price");

    subtitle->setStyle(
        LabelStyle::Small);

    subtitle->setTextTheme(
        TextTheme::DarkSecondary);

    header->addLayoutElement(
        subtitle,
        SizePolicy::Fixed,
        24);

    // --------------------------------------------------
    // Main Card
    // --------------------------------------------------

    auto breakdownCard =
        createCard();

    mainContainer->addLayoutElement(
        breakdownCard,
        SizePolicy::Fill);

    auto breakdownLayout =
        std::make_shared<Panel>();

    breakdownLayout->setLayout(
        std::make_unique<VerticalLayout>());

    breakdownLayout->setBorderVisible(false);
    breakdownLayout->setSpacing(12);

    breakdownCard->addLayoutElement(
        breakdownLayout,
        SizePolicy::Fill);

    // --------------------------------------------------
    // Title
    // --------------------------------------------------

    auto breakdownTitle =
        std::make_shared<Label>();

    breakdownTitle->setText(
        "Job Cost Summary");

    breakdownTitle->setStyle(
        LabelStyle::Normal);

    breakdownLayout->addLayoutElement(
        breakdownTitle,
        SizePolicy::Fixed,
        30);

    // --------------------------------------------------
    // Grid
    // --------------------------------------------------

    auto grid =
        std::make_shared<DataGrid>();

    grid->addColumn(
        "Description",
        300);

    grid->addColumn(
        "Cost",
        160);

    grid->addColumn(
        "Markup",
        160);

    grid->addColumn(
        "Selling Price",
        180);

    grid->addRow(
        {
            "Materials",
            viewModel.invoice.materialCost,
            "",
            ""
        });

    grid->addRow(
        {
            "Labour",
            viewModel.invoice.labourCost,
            "",
            ""
        });

    grid->addRow(
        {
            "Production",
            viewModel.invoice.productionCost,
            "",
            ""
        });

    grid->addRow(
        {
            "Subtotal",
            viewModel.invoice.subtotal,
            "",
            ""
        });

    grid->addRow(
        {
            "Markup",
            "",
            viewModel.invoice.markup,
            ""
        });

    grid->addRow(
        {
            "SELL PRICE",
            "",
            "",
            viewModel.invoice.sellPrice
        });

    breakdownLayout->addLayoutElement(
        grid,
        SizePolicy::Fill);
}

// ======================================================
// QUOTE PAGE
// ======================================================

void ResultsScreen::showQuote()
{
    clearMainContainer();

    // --------------------------------------------------
    // Quote Header
    // --------------------------------------------------

    auto header =
        std::make_shared<Panel>();

    header->setLayout(
        std::make_unique<HorizontalLayout>());

    header->setBorderVisible(false);
    header->setSpacing(20);

    mainContainer->addLayoutElement(
        header,
        SizePolicy::Fixed,
        145);

    // --------------------------------------------------
    // Company Information
    // --------------------------------------------------

    auto companyInfo =
        std::make_shared<Panel>();

    companyInfo->setLayout(
        std::make_unique<VerticalLayout>());

    companyInfo->setBorderVisible(false);
    companyInfo->setSpacing(4);

    header->addLayoutElement(
        companyInfo,
        SizePolicy::Fill);

    auto companyName =
        std::make_shared<Label>();

    companyName->setText(
        "E & G SIGNS CC");

    companyName->setStyle(
        LabelStyle::Heading);

    companyInfo->addLayoutElement(
        companyName,
        SizePolicy::Fixed,
        32);

    auto companyAddress =
        std::make_shared<Label>();

    companyAddress->setText(
        "350 Oppenheimer Street, Pinetown, 3610");

    companyAddress->setStyle(
        LabelStyle::Small);

    companyInfo->addLayoutElement(
        companyAddress,
        SizePolicy::Fixed,
        20);

    auto companyContact =
        std::make_shared<Label>();

    companyContact->setText(
        "SOUTH AFRICA");

    companyContact->setStyle(
        LabelStyle::Small);

    companyInfo->addLayoutElement(
        companyContact,
        SizePolicy::Fixed,
        20);

    auto companyPhone =
        std::make_shared<Label>();

    companyPhone->setText(
        "Tel: [Company Phone]");

    companyPhone->setStyle(
        LabelStyle::Small);

    companyInfo->addLayoutElement(
        companyPhone,
        SizePolicy::Fixed,
        20);

    auto companyEmail =
        std::make_shared<Label>();

    companyEmail->setText(
        "Email: [Company Email]");

    companyEmail->setStyle(
        LabelStyle::Small);

    companyInfo->addLayoutElement(
        companyEmail,
        SizePolicy::Fixed,
        20);

    // --------------------------------------------------
    // Quote Heading
    // --------------------------------------------------

    auto quoteHeading =
        std::make_shared<Panel>();

    quoteHeading->setLayout(
        std::make_unique<VerticalLayout>());

    quoteHeading->setBorderVisible(false);
    quoteHeading->setSpacing(2);

    header->addLayoutElement(
        quoteHeading,
        SizePolicy::Fill);

    // --------------------------------------------------
// Banking Information
// --------------------------------------------------

    auto bankingInfo =
        std::make_shared<Panel>();

    bankingInfo->setLayout(
        std::make_unique<VerticalLayout>());

    bankingInfo->setBorderVisible(false);
    bankingInfo->setSpacing(2);

    header->addLayoutElement(
        bankingInfo,
        SizePolicy::Fill);

    auto bankingTitle =
        std::make_shared<Label>();

    bankingTitle->setText(
        "BANKING DETAILS");

    bankingTitle->setStyle(
        LabelStyle::Small);

    bankingInfo->addLayoutElement(
        bankingTitle,
        SizePolicy::Fixed,
        22);

    auto bankName =
        std::make_shared<Label>();

    bankName->setText(
        "Bank: [Bank Name]");

    bankName->setStyle(
        LabelStyle::Small);

    bankingInfo->addLayoutElement(
        bankName,
        SizePolicy::Fixed,
        20);

    auto accountName =
        std::make_shared<Label>();

    accountName->setText(
        "Account: [Account Name]");

    accountName->setStyle(
        LabelStyle::Small);

    bankingInfo->addLayoutElement(
        accountName,
        SizePolicy::Fixed,
        20);

    auto accountNumber =
        std::make_shared<Label>();

    accountNumber->setText(
        "Account No: [Account Number]");

    accountNumber->setStyle(
        LabelStyle::Small);

    bankingInfo->addLayoutElement(
        accountNumber,
        SizePolicy::Fixed,
        20);

    auto branchCode =
        std::make_shared<Label>();

    branchCode->setText(
        "Branch: [Branch Code]");

    branchCode->setStyle(
        LabelStyle::Small);

    bankingInfo->addLayoutElement(
        branchCode,
        SizePolicy::Fixed,
        20);

    auto quoteTitle =
        std::make_shared<Label>();

    quoteTitle->setText(
        "COPY QUOTATION");

    quoteTitle->setStyle(
        LabelStyle::Heading);

    quoteHeading->addLayoutElement(
        quoteTitle,
        SizePolicy::Fixed,
        32);

    auto quoteSubtitle =
        std::make_shared<Label>();

    quoteSubtitle->setText(
        "PRO FORMA INVOICE");

    quoteSubtitle->setStyle(
        LabelStyle::Small);

    quoteSubtitle->setTextTheme(
        TextTheme::DarkSecondary);

    quoteHeading->addLayoutElement(
        quoteSubtitle,
        SizePolicy::Fixed,
        22);

    auto quoteNumber =
        std::make_shared<Label>();

    quoteNumber->setText(
        "QT-2026-0001");

    quoteNumber->setStyle(
        LabelStyle::Normal);

    quoteHeading->addLayoutElement(
        quoteNumber,
        SizePolicy::Fixed,
        24);

    // --------------------------------------------------
    // Quote Card
    // --------------------------------------------------

    auto quoteCard =
        createCard();

    mainContainer->addLayoutElement(
        quoteCard,
        SizePolicy::Fill);

    auto quoteLayout =
        std::make_shared<ScrollPanel>();

    quoteLayout->setLayout(
        std::make_unique<VerticalLayout>());

    quoteLayout->setBorderVisible(false);
    quoteLayout->setSpacing(10);

    quoteCard->addLayoutElement(
        quoteLayout,
        SizePolicy::Fill);

    // --------------------------------------------------
    // Customer / Quote Information
    // --------------------------------------------------

    auto infoRow =
        std::make_shared<Panel>();

    infoRow->setLayout(
        std::make_unique<HorizontalLayout>());

    infoRow->setBorderVisible(false);
    infoRow->setSpacing(30);

    quoteLayout->addLayoutElement(
        infoRow,
        SizePolicy::Fixed,
        110);

    // --------------------------------------------------
    // Customer
    // --------------------------------------------------

    auto customerInfo =
        std::make_shared<Panel>();

    customerInfo->setLayout(
        std::make_unique<VerticalLayout>());

    customerInfo->setBorderVisible(false);
    customerInfo->setSpacing(2);

    infoRow->addLayoutElement(
        customerInfo,
        SizePolicy::Fill);

    auto customerTitle =
        std::make_shared<Label>();

    customerTitle->setText(
        "CUSTOMER");

    customerTitle->setStyle(
        LabelStyle::Small);

    customerInfo->addLayoutElement(
        customerTitle,
        SizePolicy::Fixed,
        22);

    auto customerName =
        std::make_shared<Label>();

    customerName->setText(
        viewModel.invoice.customer);

    customerName->setStyle(
        LabelStyle::Normal);

    customerInfo->addLayoutElement(
        customerName,
        SizePolicy::Fixed,
        24);

    auto customerContact =
        std::make_shared<Label>();

    customerContact->setText(
        viewModel.invoice.customerContact);

    customerContact->setStyle(
        LabelStyle::Small);

    customerInfo->addLayoutElement(
        customerContact,
        SizePolicy::Fixed,
        18);


    auto customerPhone =
        std::make_shared<Label>();

    customerPhone->setText(
        viewModel.invoice.customerPhone);

    customerPhone->setStyle(
        LabelStyle::Small);

    customerInfo->addLayoutElement(
        customerPhone,
        SizePolicy::Fixed,
        18);


    auto customerEmail =
        std::make_shared<Label>();

    customerEmail->setText(
        viewModel.invoice.customerEmail);

    customerEmail->setStyle(
        LabelStyle::Small);

    customerInfo->addLayoutElement(
        customerEmail,
        SizePolicy::Fixed,
        18);

    // --------------------------------------------------
    // Quote Information
    // --------------------------------------------------

    auto quoteInfo =
        std::make_shared<Panel>();

    quoteInfo->setLayout(
        std::make_unique<VerticalLayout>());

    quoteInfo->setBorderVisible(false);
    quoteInfo->setSpacing(2);

    infoRow->addLayoutElement(
        quoteInfo,
        SizePolicy::Fill);

    auto referenceTitle =
        std::make_shared<Label>();

    referenceTitle->setText(
        "QUOTE REFERENCE");

    referenceTitle->setStyle(
        LabelStyle::Small);

    quoteInfo->addLayoutElement(
        referenceTitle,
        SizePolicy::Fixed,
        22);

    auto referenceValue =
        std::make_shared<Label>();

    referenceValue->setText(
        "QT-2026-0001");

    referenceValue->setStyle(
        LabelStyle::Normal);

    quoteInfo->addLayoutElement(
        referenceValue,
        SizePolicy::Fixed,
        28);

    // --------------------------------------------------
    // Date
    // --------------------------------------------------

    auto dateTitle =
        std::make_shared<Label>();

    dateTitle->setText(
        "DATE");

    dateTitle->setStyle(
        LabelStyle::Small);

    quoteInfo->addLayoutElement(
        dateTitle,
        SizePolicy::Fixed,
        20);

    auto dateValue =
        std::make_shared<Label>();

    dateValue->setText(
        viewModel.invoice.invoiceDate);

    dateValue->setStyle(
        LabelStyle::Small);

    quoteInfo->addLayoutElement(
        dateValue,
        SizePolicy::Fixed,
        18);

    // --------------------------------------------------
    // Quote Information Strip
    // --------------------------------------------------

    auto accountRow =
        std::make_shared<Panel>();

    accountRow->setLayout(
        std::make_unique<HorizontalLayout>());

    accountRow->setBorderVisible(false);

    accountRow->setBackgroundColour(
        SDL_Color{ 245, 245, 245, 255 });

    accountRow->setBorderVisible(true);

    accountRow->setSpacing(20);

    quoteLayout->addLayoutElement(
        accountRow,
        SizePolicy::Fixed,
        60);

    // --------------------------------------------------
    // Account
    // --------------------------------------------------

    auto accountInfo =
        std::make_shared<Panel>();

    accountInfo->setLayout(
        std::make_unique<VerticalLayout>());

    accountInfo->setBorderVisible(false);
    accountInfo->setSpacing(4);

    accountRow->addLayoutElement(
        accountInfo,
        SizePolicy::Fill);

    auto accountTitle =
        std::make_shared<Label>();

    accountTitle->setText(
        "ACCOUNT");

    accountTitle->setStyle(
        LabelStyle::Small);

    accountInfo->addLayoutElement(
        accountTitle,
        SizePolicy::Fixed,
        20);

    auto accountValue =
        std::make_shared<Label>();

    accountValue->setText(
        "-");

    accountValue->setStyle(
        LabelStyle::Normal);

    accountInfo->addLayoutElement(
        accountValue,
        SizePolicy::Fixed,
        24);

    // --------------------------------------------------
    // Your Reference
    // --------------------------------------------------

    auto yourReferenceInfo =
        std::make_shared<Panel>();

    yourReferenceInfo->setLayout(
        std::make_unique<VerticalLayout>());

    yourReferenceInfo->setBorderVisible(false);
    yourReferenceInfo->setSpacing(4);

    accountRow->addLayoutElement(
        yourReferenceInfo,
        SizePolicy::Fill);

    auto yourReferenceTitle =
        std::make_shared<Label>();

    yourReferenceTitle->setText(
        "YOUR REFERENCE");

    yourReferenceTitle->setStyle(
        LabelStyle::Small);

    yourReferenceInfo->addLayoutElement(
        yourReferenceTitle,
        SizePolicy::Fixed,
        20);

    auto yourReferenceValue =
        std::make_shared<Label>();

    yourReferenceValue->setText(
        "-");

    yourReferenceValue->setStyle(
        LabelStyle::Normal);

    yourReferenceInfo->addLayoutElement(
        yourReferenceValue,
        SizePolicy::Fixed,
        24);

    // --------------------------------------------------
    // Customer VAT Number
    // --------------------------------------------------

    auto vatInfo =
        std::make_shared<Panel>();

    vatInfo->setLayout(
        std::make_unique<VerticalLayout>());

    vatInfo->setBorderVisible(false);
    vatInfo->setSpacing(4);

    accountRow->addLayoutElement(
        vatInfo,
        SizePolicy::Fill);

    auto vatTitle =
        std::make_shared<Label>();

    vatTitle->setText(
        "CUSTOMER VAT NUMBER");

    vatTitle->setStyle(
        LabelStyle::Small);

    vatInfo->addLayoutElement(
        vatTitle,
        SizePolicy::Fixed,
        20);

    auto customerVatValue =
        std::make_shared<Label>();

    customerVatValue->setText(
        "-");

    customerVatValue->setStyle(
        LabelStyle::Normal);

    vatInfo->addLayoutElement(
        customerVatValue,
        SizePolicy::Fixed,
        24);

    // --------------------------------------------------
    // Quotation Line Items
    // --------------------------------------------------

    auto itemsTitle =
        std::make_shared<Label>();

    itemsTitle->setText(
        "QUOTATION DETAILS");

    itemsTitle->setStyle(
        LabelStyle::Normal);

    quoteLayout->addLayoutElement(
        itemsTitle,
        SizePolicy::Fixed,
        28);

    // --------------------------------------------------
    // Items Grid
    // --------------------------------------------------

    auto itemsGrid =
        std::make_shared<DataGrid>();

    itemsGrid->addColumn(
        "Code",
        90);

    itemsGrid->addColumn(
        "Description",
        360);

    itemsGrid->addColumn(
        "Qty",
        60);

    itemsGrid->addColumn(
        "Unit",
        70);

    itemsGrid->addColumn(
        "Area",
        90);

    itemsGrid->addColumn(
        "Cost / Unit",
        110);

    itemsGrid->addColumn(
        "Net Price",
        120);

    // --------------------------------------------------
    // Populate Items
    // --------------------------------------------------

    int itemNumber = 1;

    for (const auto& job :
        viewModel.invoice.jobs)
    {
        std::string description;

        if (job.isRoll)
        {
            description =
                job.variant +
                " - " +
                job.rollWidth +
                "mm x " +
                job.lengthUsed +
                "m";
        }
        else
        {
            description =
                job.variant +
                " - " +
                job.sheetSize;
        }

        std::string code =
            "ITEM-" +
            std::to_string(itemNumber);

        itemsGrid->addRow(
            {
                code,
                description,
                job.quantity,
                "EACH",
                job.area,
                "R -",
                job.sellPrice
            });

        itemNumber++;
    }

    quoteLayout->addLayoutElement(
        itemsGrid,
        SizePolicy::Fixed,
        36 +
        static_cast<int>(viewModel.invoice.jobs.size()) * 52);

    // --------------------------------------------------
    // Totals
    // --------------------------------------------------

    auto totals =
        std::make_shared<Panel>();

    totals->setLayout(
        std::make_unique<VerticalLayout>());

    totals->setBorderVisible(false);
    totals->setSpacing(4);

    quoteLayout->addLayoutElement(
        totals,
        SizePolicy::Fixed,
        125);

    // --------------------------------------------------
    // Sub Total
    // --------------------------------------------------

    auto subtotalRow =
        std::make_shared<Panel>();

    subtotalRow->setLayout(
        std::make_unique<HorizontalLayout>());

    subtotalRow->setBorderVisible(false);

    totals->addLayoutElement(
        subtotalRow,
        SizePolicy::Fixed,
        24);

    auto subtotalLabel =
        std::make_shared<Label>();

    subtotalLabel->setText(
        "SUB TOTAL");

    subtotalLabel->setStyle(
        LabelStyle::Small);

    subtotalRow->addLayoutElement(
        subtotalLabel,
        SizePolicy::Fixed,
        150);

    auto subtotalValue =
        std::make_shared<Label>();

    subtotalValue->setText(
        viewModel.invoice.sellPrice);

    subtotalValue->setStyle(
        LabelStyle::Small);

    subtotalRow->addLayoutElement(
        subtotalValue,
        SizePolicy::Fill);

    // --------------------------------------------------
    // VAT
    // --------------------------------------------------

    auto vatRow =
        std::make_shared<Panel>();

    vatRow->setLayout(
        std::make_unique<HorizontalLayout>());

    vatRow->setBorderVisible(false);

    totals->addLayoutElement(
        vatRow,
        SizePolicy::Fixed,
        24);

    auto vatLabel =
        std::make_shared<Label>();

    vatLabel->setText(
        "VAT 15%");

    vatLabel->setStyle(
        LabelStyle::Small);

    vatRow->addLayoutElement(
        vatLabel,
        SizePolicy::Fixed,
        150);

    auto vatValue =
        std::make_shared<Label>();

    vatValue->setText(
        viewModel.invoice.vat);

    vatValue->setStyle(
        LabelStyle::Small);

    vatRow->addLayoutElement(
        vatValue,
        SizePolicy::Fixed,
        150);

    // --------------------------------------------------
    // Total
    // --------------------------------------------------

    auto totalRow =
        std::make_shared<Panel>();

    totalRow->setLayout(
        std::make_unique<HorizontalLayout>());

    totalRow->setBorderVisible(false);

    totals->addLayoutElement(
        totalRow,
        SizePolicy::Fixed,
        42);

    auto totalLabel =
        std::make_shared<Label>();

    totalLabel->setText(
        "TOTAL");

    totalLabel->setStyle(
        LabelStyle::Heading);

    totalRow->addLayoutElement(
        totalLabel,
        SizePolicy::Fixed,
        150);

    auto totalValue =
        std::make_shared<Label>();

    totalValue->setText(
        viewModel.invoice.total);

    totalValue->setStyle(
        LabelStyle::Heading);

    totalRow->addLayoutElement(
        totalValue,
        SizePolicy::Fixed,
        150);

    auto scopeTitle =
        std::make_shared<Label>();

    scopeTitle->setText(
        "SCOPE / NOTES");

    scopeTitle->setStyle(
        LabelStyle::Normal);

    quoteLayout->addLayoutElement(
        scopeTitle,
        SizePolicy::Fixed,
        28);

    auto scopePanel =
        std::make_shared<Panel>();

    scopePanel->setBorderVisible(
        true);

    scopePanel->setLayout(
        std::make_unique<VerticalLayout>());

    quoteLayout->addLayoutElement(
        scopePanel,
        SizePolicy::Fixed,
        70);

    auto scopeText =
        std::make_shared<Label>();

    scopeText->setText(
        "Quotation based on the specifications and quantities listed above.");

    scopeText->setStyle(
        LabelStyle::Small);

    scopePanel->addLayoutElement(
        scopeText,
        SizePolicy::Fill);

    auto termsTitle =
        std::make_shared<Label>();

    termsTitle->setText(
        "TERMS & CONDITIONS");

    termsTitle->setStyle(
        LabelStyle::Normal);

    quoteLayout->addLayoutElement(
        termsTitle,
        SizePolicy::Fixed,
        28);

    auto termsPanel =
        std::make_shared<Panel>();

    termsPanel->setBorderVisible(
        true);

    termsPanel->setLayout(
        std::make_unique<VerticalLayout>());

    quoteLayout->addLayoutElement(
        termsPanel,
        SizePolicy::Fixed,
        90);

    auto termsText =
        std::make_shared<Label>();

    termsText->setText(
        "Prices are subject to the specifications stated in this quotation. "
        "Production will commence once the quotation has been accepted. "
        "Payment terms are subject to the customer's approved account terms.");

    termsText->setStyle(
        LabelStyle::Small);

    termsPanel->addLayoutElement(
        termsText,
        SizePolicy::Fill);

    auto acceptanceTitle =
        std::make_shared<Label>();

    acceptanceTitle->setText(
        "ACCEPTANCE OF QUOTATION");

    acceptanceTitle->setStyle(
        LabelStyle::Normal);

    quoteLayout->addLayoutElement(
        acceptanceTitle,
        SizePolicy::Fixed,
        28);

    auto acceptancePanel =
        std::make_shared<Panel>();

    acceptancePanel->setBorderVisible(
        true);

    acceptancePanel->setLayout(
        std::make_unique<VerticalLayout>());

    acceptancePanel->setSpacing(4);

    quoteLayout->addLayoutElement(
        acceptancePanel,
        SizePolicy::Fixed,
        125);

    auto acceptanceText =
        std::make_shared<Label>();

    acceptanceText->setText(
        "I/We hereby accept this quotation and authorise "
        "the work described above to proceed.");

    acceptanceText->setStyle(
        LabelStyle::Small);

    acceptancePanel->addLayoutElement(
        acceptanceText,
        SizePolicy::Fixed,
        24);

    auto acceptanceCustomerName =
        std::make_shared<Label>();

    acceptanceCustomerName->setText(
        "Customer Name: ______________________________");

    acceptanceCustomerName->setStyle(
        LabelStyle::Small);

    acceptancePanel->addLayoutElement(
        acceptanceCustomerName,
        SizePolicy::Fixed,
        22);

    auto signature =
        std::make_shared<Label>();

    signature->setText(
        "Signature:          ______________________________");

    signature->setStyle(
        LabelStyle::Small);

    acceptancePanel->addLayoutElement(
        signature,
        SizePolicy::Fixed,
        22);

    auto acceptanceDate =
        std::make_shared<Label>();

    acceptanceDate->setText(
        "Date:                 ______________________________");

    acceptanceDate->setStyle(
        LabelStyle::Small);

    acceptancePanel->addLayoutElement(
        acceptanceDate,
        SizePolicy::Fixed,
        22);
}