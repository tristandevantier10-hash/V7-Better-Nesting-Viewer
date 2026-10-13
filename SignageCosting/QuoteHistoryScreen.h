#pragma once

#include "Screen.h"
#include "DataGrid.h"
#include "TextBox.h"
#include "Label.h"
#include "Button.h"
#include <functional>
#include <vector>
#include "QuoteRecord.h"

class QuoteHistoryScreen : public Screen
{
public:
    QuoteHistoryScreen();
    void update(const SDL_Event& e) override;
    void refreshQuotes();
    void setBackCallback(std::function<void()> callback);
    void setQuoteSelectedCallback(std::function<void(const QuoteRecord&)> callback);
    void setViewOverviewCallback(std::function<void(const QuoteRecord&)> callback);

private:
    std::shared_ptr<TextBox> searchBox;
    std::shared_ptr<DataGrid> quoteGrid;
    std::shared_ptr<DataGrid> detailGrid;
    std::shared_ptr<Label> countLabel;
    std::shared_ptr<Button> backButton;
    std::shared_ptr<Button> overviewButton;
    std::function<void()> backCallback;
    std::function<void(const QuoteRecord&)> quoteSelectedCallback;
    std::function<void(const QuoteRecord&)> viewOverviewCallback;
    std::vector<int> visibleQuoteIndices;
    int selectedQuoteRow = -1;
    void filterQuotes(const std::string& query);
    void showQuote(int row);
};
