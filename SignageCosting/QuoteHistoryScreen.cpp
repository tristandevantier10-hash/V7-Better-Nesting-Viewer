#include "QuoteHistoryScreen.h"
#include "QuoteDatabase.h"
#include "VerticalLayout.h"
#include "HorizontalLayout.h"
#include "Panel.h"
#include "Metrics.h"
#include "PageHeader.h"
#include "Renderer.h"
#include <SDL2/SDL_image.h>
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Data.Pdf.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>
#pragma comment(lib, "windowsapp.lib")

class QuotePdfPreviewPanel : public Panel
{
    struct LoadState
    {
        std::mutex mutex;
        std::size_t generation = 0;
        bool complete = false;
        std::vector<uint8_t> pngBytes;
        std::string error;
    };

public:
    void setPdfPath(const std::string& newPath)
    {
        if (path == newPath)
            return;
        path = newPath;
        textureAttempted = false;
        pageScrollY = 0;
        message = path.empty() ? "PDF preview is unavailable for this quote." : "Loading PDF preview...";
        if (texture)
        {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }

        std::size_t requestGeneration = 0;
        {
            std::lock_guard<std::mutex> lock(loadState->mutex);
            requestGeneration = ++loadState->generation;
            loadState->complete = path.empty();
            loadState->pngBytes.clear();
            loadState->error.clear();
        }
        if (path.empty())
            return;

        const std::string pathToLoad = path;
        const uint32_t renderWidth = static_cast<uint32_t>(
            std::clamp(bounds.w * 3, 1800, 4096));

        const auto state = loadState;
        std::thread([state, pathToLoad, requestGeneration, renderWidth]
        {
            std::vector<uint8_t> bytes;
            std::string errorMessage;
            bool apartmentInitialized = false;
            try
            {
                winrt::init_apartment(winrt::apartment_type::multi_threaded);
                apartmentInitialized = true;

                const auto file = winrt::Windows::Storage::StorageFile::GetFileFromPathAsync(
                    winrt::to_hstring(pathToLoad)).get();
                const auto document = winrt::Windows::Data::Pdf::PdfDocument::LoadFromFileAsync(file).get();
                if (document.PageCount() == 0)
                    throw std::runtime_error("The PDF has no pages.");

                const auto stream = winrt::Windows::Storage::Streams::InMemoryRandomAccessStream();
                const auto page = document.GetPage(0);
                winrt::Windows::Data::Pdf::PdfPageRenderOptions renderOptions;
                renderOptions.DestinationWidth(renderWidth);
                page.RenderToStreamAsync(stream, renderOptions).get();
                winrt::Windows::Storage::Streams::DataReader reader(stream.GetInputStreamAt(0));
                const auto byteCount = static_cast<uint32_t>(stream.Size());
                reader.LoadAsync(byteCount).get();
                bytes.resize(byteCount);
                reader.ReadBytes(bytes);
            }
            catch (const std::exception& exception)
            {
                errorMessage = exception.what();
            }
            catch (...)
            {
                errorMessage = "The PDF page could not be rendered.";
            }
            if (apartmentInitialized)
                winrt::uninit_apartment();

            std::lock_guard<std::mutex> lock(state->mutex);
            if (state->generation == requestGeneration)
            {
                state->pngBytes = std::move(bytes);
                state->error = std::move(errorMessage);
                state->complete = true;
            }
        }).detach();
    }

    void update(const SDL_Event& event) override
    {
        Panel::update(event);
        if (event.type != SDL_MOUSEWHEEL || !texture)
            return;

        int mouseX = 0;
        int mouseY = 0;
        SDL_GetMouseState(&mouseX, &mouseY);
        const SDL_Point mouse{ mouseX, mouseY };
        if (!SDL_PointInRect(&mouse, &bounds))
            return;

        int textureWidth = 0;
        int textureHeight = 0;
        if (SDL_QueryTexture(texture, nullptr, nullptr, &textureWidth, &textureHeight) != 0 ||
            textureWidth <= 0 || textureHeight <= 0)
            return;

        const int pageWidth = (std::max)(1, bounds.w - 24);
        const int pageHeight = static_cast<int>(
            static_cast<double>(pageWidth) * textureHeight / textureWidth);
        const int maxScroll = (std::max)(0, pageHeight - (bounds.h - 24));
        pageScrollY = (std::clamp)(pageScrollY - event.wheel.y * 60, 0, maxScroll);
    }

    void render(Renderer& renderer) override
    {
        performLayout();
        setBackgroundColour({ 250, 250, 250, 255 });
        Panel::render(renderer);
        loadTextureWhenReady(renderer);

        if (texture)
        {
            int textureWidth = 0;
            int textureHeight = 0;
            SDL_QueryTexture(texture, nullptr, nullptr, &textureWidth, &textureHeight);
            const int pageWidth = (std::max)(1, bounds.w - 24);
            const int pageHeight = static_cast<int>(
                static_cast<double>(pageWidth) * textureHeight / textureWidth);
            const int maxScroll = (std::max)(0, pageHeight - (bounds.h - 24));
            pageScrollY = (std::clamp)(pageScrollY, 0, maxScroll);

            const SDL_Rect clip = bounds;
            renderer.pushClip(clip);
            const SDL_Rect pageArea{
                bounds.x + 12,
                bounds.y + 12 - pageScrollY,
                pageWidth,
                pageHeight
            };
            renderer.fillRect(pageArea, { 255, 255, 255, 255 });
            renderer.drawTexture(texture, pageArea, false);
            renderer.popClip();
        }
        else
            renderer.drawText(message, bounds.x + 14, bounds.y + 16, { 105, 105, 105, 255 });
    }

private:
    void loadTextureWhenReady(Renderer& renderer)
    {
        if (path.empty() || texture || textureAttempted)
            return;

        std::lock_guard<std::mutex> lock(loadState->mutex);
        if (!loadState->complete)
            return;

        textureAttempted = true;
        if (loadState->pngBytes.empty())
        {
            message = loadState->error.empty()
                ? "PDF preview could not be loaded."
                : loadState->error;
            return;
        }

        SDL_RWops* imageStream = SDL_RWFromConstMem(
            loadState->pngBytes.data(), static_cast<int>(loadState->pngBytes.size()));
        if (!imageStream)
        {
            message = "Could not read the rendered PDF page.";
            return;
        }
        SDL_Surface* surface = IMG_Load_RW(imageStream, 1);
        if (!surface)
        {
            message = "Could not decode the rendered PDF page.";
            return;
        }
        texture = SDL_CreateTextureFromSurface(renderer.getSDLRenderer(), surface);
        SDL_FreeSurface(surface);
        message = texture ? "Page 1" : "Could not display the PDF page.";
    }

    std::string path;
    std::string message = "Select a quote to preview its PDF";
    SDL_Texture* texture = nullptr;
    bool textureAttempted = false;
    int pageScrollY = 0;
    std::shared_ptr<LoadState> loadState = std::make_shared<LoadState>();
};

namespace
{
    std::string lower(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return value;
    }
}

QuoteHistoryScreen::QuoteHistoryScreen(std::shared_ptr<NavigationPanel> sharedNavigation)
{
    setLayout(std::make_unique<HorizontalLayout>());
    setPadding(0);
    setSpacing(0);

    auto navigation = std::move(sharedNavigation);
    addLayoutElement(navigation, SizePolicy::Fixed, Metrics::SidebarWidth);

    auto content = std::make_shared<Panel>();
    content->setLayout(std::make_unique<VerticalLayout>());
    content->setPadding(0);
    content->setSpacing(0);

    auto header = std::make_shared<DashboardBannerPanel>();
    header->setLayout(std::make_unique<HorizontalLayout>());
    configurePageHeader(header);
    auto title = std::make_shared<Label>();
    title->setText("Quotes");
    title->setStyle(LabelStyle::Heading);
    configurePageHeaderTitleLabel(title);
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
    content->addLayoutElement(header, SizePolicy::Fixed, 132);

    auto body = std::make_shared<Panel>();
    body->setLayout(std::make_unique<VerticalLayout>());
    body->setPadding(Metrics::SpaceM);
    body->setSpacing(Metrics::SpaceS);
    content->addLayoutElement(body, SizePolicy::Fill);
    addLayoutElement(content, SizePolicy::Fill);

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
    body->addLayoutElement(quoteGrid, SizePolicy::Fixed, 145);

    auto detailTitle = std::make_shared<Label>();
    detailTitle->setText("SELECTED QUOTE");
    detailTitle->setStyle(LabelStyle::Small);
    body->addLayoutElement(detailTitle, SizePolicy::Fixed, Metrics::SmallRow);

    statusRow = std::make_shared<Panel>();
    statusRow->setLayout(std::make_unique<HorizontalLayout>());
    statusRow->setBorderVisible(false);
    statusRow->setPadding(0);
    statusRow->setSpacing(Metrics::SpaceM);

    statusLabel = std::make_shared<Label>();
    statusLabel->setText("Select a quote to change its status");
    statusLabel->setStyle(LabelStyle::Small);
    statusRow->addLayoutElement(statusLabel, SizePolicy::Fill);

    statusCombo = std::make_shared<ComboBox>();
    for (const auto& status : availableStatuses)
        statusCombo->addItem(status);
    statusCombo->setEnabled(false);
    statusCombo->setVisible(false);
    statusRow->setVisible(false);
    statusCombo->setSelectionChangedCallback([this](int index)
    {
        updateSelectedQuoteStatus(index);
    });
    statusRow->addLayoutElement(statusCombo, SizePolicy::Fixed, 180);
    body->addLayoutElement(statusRow, SizePolicy::Fixed, Metrics::ControlHeight);

    detailGrid = std::make_shared<DataGrid>();
    detailGrid->addColumn("Quote details", 170);
    detailGrid->addColumn("Value", 650);
    detailGrid->setRowHeight(Metrics::NormalRow);
    detailGrid->setTextStyle(LabelStyle::Small);
    body->addLayoutElement(detailGrid, SizePolicy::Fixed, 155);

    pdfPreviewMessage = std::make_shared<Label>();
    pdfPreviewMessage->setText("PDF PREVIEW  ·  PAGE 1");
    pdfPreviewMessage->setStyle(LabelStyle::Small);
    pdfPreviewMessage->setVisible(false);
    body->addLayoutElement(pdfPreviewMessage, SizePolicy::Fixed, Metrics::SmallRow);

    pdfPreview = std::make_shared<QuotePdfPreviewPanel>();
    pdfPreview->setVisible(false);
    body->addLayoutElement(pdfPreview, SizePolicy::Fill);
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

void QuoteHistoryScreen::setQuoteSelectedCallback(
    std::function<std::string(const QuoteRecord&)> callback)
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
    statusCombo->setEnabled(false);
    statusCombo->setVisible(false);
    statusRow->setVisible(false);
    statusLabel->setText("Select a quote to change its status");
    pdfPreview->setPdfPath({});
    pdfPreview->setVisible(false);
    pdfPreviewMessage->setVisible(false);
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

    updateSelectedQuoteDetails(true);
}

void QuoteHistoryScreen::updateSelectedQuoteDetails(bool notifySelection)
{
    if (selectedQuoteRow < 0 || selectedQuoteRow >= static_cast<int>(visibleQuoteIndices.size()))
        return;

    const auto& record = QuoteDatabase::getAll()[visibleQuoteIndices[selectedQuoteRow]];
    const auto& invoice = record.invoice;
    auto status = std::find(availableStatuses.begin(), availableStatuses.end(), record.status);
    if (status == availableStatuses.end())
    {
        availableStatuses.push_back(record.status);
        statusCombo->addItem(record.status);
        status = availableStatuses.end() - 1;
    }
    statusCombo->setSelectedIndex(static_cast<int>(status - availableStatuses.begin()));
    statusCombo->setEnabled(true);
    statusCombo->setVisible(true);
    statusRow->setVisible(true);
    statusLabel->setText("Change status");

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
    if (notifySelection && quoteSelectedCallback)
    {
        pdfPreview->setPdfPath(quoteSelectedCallback(record));
        pdfPreview->setVisible(true);
        pdfPreviewMessage->setVisible(true);
    }
}

void QuoteHistoryScreen::updateSelectedQuoteStatus(int statusIndex)
{
    if (selectedQuoteRow < 0 || selectedQuoteRow >= static_cast<int>(visibleQuoteIndices.size()) ||
        statusIndex < 0 || statusIndex >= static_cast<int>(availableStatuses.size()))
        return;

    const int quoteIndex = visibleQuoteIndices[selectedQuoteRow];
    const QuoteRecord& current = QuoteDatabase::getAll()[quoteIndex];
    const std::string previousStatus = current.status;
    const std::string newStatus = availableStatuses[statusIndex];
    if (newStatus == previousStatus)
        return;

    if (!QuoteDatabase::updateStatus(current.invoice.jobref, newStatus))
    {
        auto previous = std::find(availableStatuses.begin(), availableStatuses.end(), previousStatus);
        if (previous != availableStatuses.end())
            statusCombo->setSelectedIndex(static_cast<int>(previous - availableStatuses.begin()));
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR,
            "Status Update Failed",
            "The quote status could not be saved. Please try again.",
            nullptr);
        return;
    }

    const QuoteRecord& updated = QuoteDatabase::getAll()[quoteIndex];
    quoteGrid->updateRow(selectedQuoteRow, {
        updated.invoice.jobref.empty() ? "(no reference)" : updated.invoice.jobref,
        updated.invoice.customer,
        updated.invoice.invoiceDate,
        updated.status,
        updated.invoice.total
    });
    updateSelectedQuoteDetails(false);
}
