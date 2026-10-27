#pragma once

#include "Screen.h"
#include "DataGrid.h"
#include "TextBox.h"
#include "Label.h"
#include "Button.h"
#include "ComboBox.h"
#include <functional>
#include <vector>
#include "QuoteRecord.h"
#include "NavigationPanel.h"

class QuotePdfPreviewPanel;

class QuoteHistoryScreen : public Screen
{
public:
    explicit QuoteHistoryScreen(std::shared_ptr<NavigationPanel> sharedNavigation);
    void update(const SDL_Event& e) override;
    void refreshQuotes();
    void setBackCallback(std::function<void()> callback);
    void setQuoteSelectedCallback(std::function<std::string(const QuoteRecord&)> callback);
    void setViewOverviewCallback(std::function<void(const QuoteRecord&)> callback);

private:
    std::shared_ptr<TextBox> searchBox;
    std::shared_ptr<DataGrid> quoteGrid;
    std::shared_ptr<DataGrid> detailGrid;
    std::shared_ptr<QuotePdfPreviewPanel> pdfPreview;
    std::shared_ptr<Label> pdfPreviewMessage;
    std::shared_ptr<Label> countLabel;
    std::shared_ptr<Button> backButton;
    std::shared_ptr<Button> overviewButton;
    std::shared_ptr<Panel> statusRow;
    std::shared_ptr<Label> statusLabel;
    std::shared_ptr<ComboBox> statusCombo;
    std::function<void()> backCallback;
    std::function<std::string(const QuoteRecord&)> quoteSelectedCallback;
    std::function<void(const QuoteRecord&)> viewOverviewCallback;
    std::vector<int> visibleQuoteIndices;
    std::vector<std::string> availableStatuses{
        "Draft", "Sent", "Accepted", "Declined", "Expired"
    };
    int selectedQuoteRow = -1;
    void filterQuotes(const std::string& query);
    void showQuote(int row);
    void updateSelectedQuoteDetails(bool notifySelection);
    void updateSelectedQuoteStatus(int statusIndex);
};
