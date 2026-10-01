#pragma once

#include "Screen.h"
#include "Panel.h"
#include "Button.h"
#include "Label.h"
#include "DataGrid.h"
#include "InvoiceViewModel.h"
#include "ResultsViewModel.h"
#include "SheetPreviewPanel.h"
#include <functional>
#include <memory>
#include <vector>
#include "NavigationPanel.h"

class Renderer;

class ResultsScreen : public Screen
{
public:

    ResultsScreen();

    void update(const SDL_Event& e) override;
    void render(Renderer& renderer) override;

    void setViewModel(
        const ResultsViewModel& vm);

    void setSheets(
        const std::vector<Sheet>& sheets);

    // --------------------------------------------------
    // Navigation callbacks
    // --------------------------------------------------

    void setNewJobCallback(
        std::function<void()> callback);

    void setBackCallback(
        std::function<void()> callback);

    void setPrintCallback(
        std::function<void()> callback);

    void setExportCallback(
        std::function<void()> callback);

    void setOverviewCallback(
        std::function<void()> callback);

    void setNestingSheetsCallback(
        std::function<void()> callback);

    void setCostBreakdownCallback(
        std::function<void()> callback);

    void setQuoteCallback(
        std::function<void()> callback);

private:

    // --------------------------------------------------
    // Main layout
    // --------------------------------------------------

    std::shared_ptr<Panel> contentPanel;
    std::shared_ptr<NavigationPanel> sidebar;
    std::shared_ptr<Panel> mainContainer;

    // --------------------------------------------------
    // Header
    // --------------------------------------------------

    std::shared_ptr<Label> pageTitle;
    std::shared_ptr<Label> pageSubtitle;

    std::shared_ptr<Panel> customerCard;

    // --------------------------------------------------
    // KPI cards
    // --------------------------------------------------

    std::shared_ptr<Label> dateSummaryLabel;
    std::shared_ptr<Panel> totalPriceCard;
    std::shared_ptr<Panel> totalCostCard;
    std::shared_ptr<Panel> grossProfitCard;
    std::shared_ptr<Panel> itemsCard;
    std::shared_ptr<Panel> sheetsCard;
    std::shared_ptr<Label> totalPriceValueLabel;
    std::shared_ptr<Label> totalCostValueLabel;
    std::shared_ptr<Label> grossProfitValueLabel;
    std::shared_ptr<Label> itemsValueLabel;
    std::shared_ptr<Label> sheetsValueLabel;

    // ==================================================
    // JOB / CLIENT INFORMATION
    // ==================================================

    std::shared_ptr<Label> jobReferenceLabel;
    std::shared_ptr<Label> jobReferenceValueLabel;

    std::shared_ptr<Label> dateLabel;
    std::shared_ptr<Label> dateValueLabel;

    std::shared_ptr<Label> paymentTermsLabel;
    std::shared_ptr<Label> paymentTermsValueLabel;

    std::shared_ptr<Label> validForLabel;
    std::shared_ptr<Label> validForValueLabel;

    // --------------------------------------------------
    // Main content
    // --------------------------------------------------

    std::shared_ptr<Panel> leftColumn;
    std::shared_ptr<Panel> rightColumn;

    std::shared_ptr<Panel> costBreakdownCard;
    std::shared_ptr<Panel> jobItemsCard;
    std::shared_ptr<Panel> clientInfoCard;
    std::shared_ptr<Panel> nestingCard;

    std::shared_ptr<DataGrid> costBreakdownGrid;
    std::shared_ptr<DataGrid> jobItemsGrid;

    std::shared_ptr<SheetPreviewPanel> previewPanel;

    // --------------------------------------------------
    // Bottom buttons
    // --------------------------------------------------

    std::shared_ptr<Button> newJobButton;
    std::shared_ptr<Button> printButton;
    std::shared_ptr<Button> exportButton;

    // --------------------------------------------------
    // Data
    // --------------------------------------------------

    ResultsViewModel viewModel;

    // --------------------------------------------------
    // Callbacks
    // --------------------------------------------------

    std::function<void()> newJobCallback;
    std::function<void()> backCallback;
    std::function<void()> printCallback;
    std::function<void()> exportCallback;

    std::vector<Sheet> nestingSheets;

    std::function<void()> overviewCallback;
    std::function<void()> nestingSheetsCallback;
    std::function<void()> costBreakdownCallback;
    std::function<void()> quoteCallback;

    // --------------------------------------------------
    // Construction helpers
    // --------------------------------------------------

    void buildSidebar();
    void buildHeader();
    void buildKpiRow();
    void buildContentArea();
    void buildBottomActions();

    void showOverview();
    void showNestingSheets();
    void showCostBreakdown();
    void showQuote();

    void clearMainContainer();

    std::shared_ptr<Panel> createKpiCard(
        const std::string& title,
        const std::string& value,
        const std::string& subtitle,
        std::shared_ptr<Label>* valueLabelOut = nullptr);

    std::shared_ptr<Panel> createCard();

    void refreshView();

    void refreshCostBreakdown();
    void refreshJobItems();
};