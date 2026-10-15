#include "QuoteHistoryScreen.h"
#include "QuoteDatabase.h"
#include "VerticalLayout.h"
#include "HorizontalLayout.h"
#include "Panel.h"
#include "Metrics.h"
#include "PageHeader.h"
#include <algorithm>
#include <cctype>

namespace
{
    std::string lower(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return value;
    }
}

QuoteHistoryScreen::QuoteHistoryScreen()
{
    setLayout(std::make_unique<VerticalLayout>());
    auto header = std::make_shared<PageHeaderPanel>();
    header->setLayout(std::make_unique<HorizontalLayout>());
    configurePageHeader(header);
    addPageHeaderLogo(header);
    auto title = std::make_shared<Label>();
    title->setText("Quotes");
    title->setStyle(LabelStyle::Heading);
    configurePageHeaderTitleLabel(title);
    title->setAlignment(LabelAlignment::Center);
    header->addLayoutElement(title, SizePolicy::Fill);
    countLabel = std::make_shared<Label>();
    countLabel->setStyle(LabelStyle::Small);
    configurePageHeaderSubtitleLabel(countLabel);
    header->addLayoutElement(countLabel, SizePolicy::Fixed, 140);
    backButton = std::make_shared<Button>();
    backButton->setText("Back");
    backButton->setOnClick([this]() { if (backCallback) backCallback(); });
    header->addLayoutElement(backButton, SizePolicy::Fixed, 100);
    overviewButton = std::make_shared<Button>();
    overviewButton->setText("View overview");
    overviewButton->setOnClick([this]()
    {
        const int row = selectedQuoteRow;
        if (row >= 0 && row < static_cast<int>(visibleQuoteIndices.size()) && viewOverviewCallback)
            viewOverviewCallback(QuoteDatabase::getAll()[visibleQuoteIndices[row]]);
    });
    header->addLayoutElement(overviewButton, SizePolicy::Fixed, 150);
    addLayoutElement(header, SizePolicy::Fixed, Metrics::HeaderHeight);

    auto pageContent = std::make_shared<Panel>();
    pageContent->setLayout(std::make_unique<HorizontalLayout>());
    addLayoutElement(pageContent, SizePolicy::Fill);

    auto navigation = createPrimaryNavigationPanel(
        PrimaryNavigationRoute::Quotes,
        [this](PrimaryNavigationRoute route)
        {
            if (navigationCallback)
                navigationCallback(route);
        });
    pageContent->addLayoutElement(
        navigation,
        SizePolicy::Fixed,
        Metrics::SidebarWidth);

    auto body = std::make_shared<Panel>();
    body->setLayout(std::make_unique<VerticalLayout>());
    body->setPadding(Metrics::SpaceL);
    body->setSpacing(Metrics::SpaceM);
    pageContent->addLayoutElement(body, SizePolicy::Fill);

    searchBox = std::make_shared<TextBox>();
    searchBox->setPlaceholder("Search by quote reference, customer, contact, status or date...");
    searchBox->setTextChangedCallback([this](const std::string& query) { filterQuotes(query); });
    body->addLayoutElement(searchBox, SizePolicy::Fixed, Metrics::ControlHeight);

    auto listTitle = std::make_shared<Label>();
    listTitle->setText("QUOTE HISTORY");
    listTitle->setStyle(LabelStyle::Small);
    body->addLayoutElement(listTitle, SizePolicy::Fixed, Metrics::SmallRow);

    quoteGrid = std::make_shared<DataGrid>();
    quoteGrid->addColumn("Reference", 180);
    quoteGrid->addColumn("Customer", 210);
    quoteGrid->addColumn("Date", 140);
    quoteGrid->addColumn("Status", 110);
    quoteGrid->addColumn("Total", 130);
    quoteGrid->setRowHeight(Metrics::NormalRow);
    quoteGrid->setTextStyle(LabelStyle::Small);
    quoteGrid->setSelectionChangedCallback([this](int row) { showQuote(row); });
    body->addLayoutElement(quoteGrid, SizePolicy::Fill);

    auto detailTitle = std::make_shared<Label>();
    detailTitle->setText("SELECTED QUOTE");
    detailTitle->setStyle(LabelStyle::Small);
    body->addLayoutElement(detailTitle, SizePolicy::Fixed, Metrics::SmallRow);

    detailGrid = std::make_shared<DataGrid>();
    detailGrid->addColumn("Quote details", 170);
    detailGrid->addColumn("Value", 650);
    detailGrid->setRowHeight(Metrics::NormalRow);
    detailGrid->setTextStyle(LabelStyle::Small);
    body->addLayoutElement(detailGrid, SizePolicy::Fixed, 190);
    refreshQuotes();
}

void QuoteHistoryScreen::update(const SDL_Event& e)
{
    Screen::update(e);
}

void QuoteHistoryScreen::refreshQuotes()
{
    filterQuotes(searchBox ? searchBox->getText() : std::string{});
}

void QuoteHistoryScreen::setBackCallback(std::function<void()> callback)
{
    backCallback = callback;
}

void QuoteHistoryScreen::setNavigationCallback(
    std::function<void(PrimaryNavigationRoute)> callback)
{
    navigationCallback = callback;
}

void QuoteHistoryScreen::setQuoteSelectedCallback(
    std::function<void(const QuoteRecord&)> callback)
{
    quoteSelectedCallback = callback;
}

void QuoteHistoryScreen::setViewOverviewCallback(
    std::function<void(const QuoteRecord&)> callback)
{
    viewOverviewCallback = callback;
}

void QuoteHistoryScreen::filterQuotes(const std::string& query)
{
    quoteGrid->clear();
    visibleQuoteIndices.clear();
    selectedQuoteRow = -1;
    const std::string needle = lower(query);
    const auto& quotes = QuoteDatabase::getAll();
    for (int i = static_cast<int>(quotes.size()) - 1; i >= 0; --i)
    {
        const auto& q = quotes[i];
        const auto& invoice = q.invoice;
        std::string searchable = invoice.jobref + " " + invoice.customer + " " +
            invoice.customerContact + " " + invoice.customerPhone + " " +
            invoice.customerEmail + " " + q.status + " " + invoice.invoiceDate;
        for (const auto& job : invoice.jobs)
            searchable += " " + job.material + " " + job.variant;
        if (!needle.empty() && lower(searchable).find(needle) == std::string::npos)
            continue;
        visibleQuoteIndices.push_back(i);
        quoteGrid->addRow({ invoice.jobref.empty() ? "(no reference)" : invoice.jobref,
            invoice.customer, invoice.invoiceDate, q.status, invoice.total }, i);
    }
    countLabel->setText(std::to_string(visibleQuoteIndices.size()) + " quotes");
    detailGrid->clear();
}

void QuoteHistoryScreen::showQuote(int row)
{
    if (row < 0 || row >= static_cast<int>(visibleQuoteIndices.size()))
        return;
    selectedQuoteRow = row;
    const auto& record = QuoteDatabase::getAll()[visibleQuoteIndices[row]];
    const auto& invoice = record.invoice;
    detailGrid->clear();
    auto add = [this](const std::string& name, const std::string& value)
    {
        detailGrid->addRow({ name, value });
    };
    add("Reference / status", invoice.jobref + "  /  " + record.status);
    add("Customer", invoice.customer + (invoice.customerContact.empty() ? "" : " / " + invoice.customerContact));
    add("Contact", invoice.customerPhone + (invoice.customerEmail.empty() ? "" : " / " + invoice.customerEmail));
    add("Date", invoice.invoiceDate);
    std::string items;
    for (const auto& job : invoice.jobs)
    {
        if (!items.empty()) items += " | ";
        items += job.material;
        if (!job.variant.empty()) items += " " + job.variant;
        items += " x" + job.quantity;
    }
    add("Items", items.empty() ? "No item details" : items);
    add("Subtotal / VAT", invoice.subtotal + " / " + invoice.vat);
    add("Total", invoice.total);
    if (quoteSelectedCallback)
        quoteSelectedCallback(record);
}
