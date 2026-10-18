#pragma once

#include "UIElement.h"
#include "QuoteRecord.h"
#include <functional>
#include <string>
#include <vector>

class DashboardContent : public UIElement
{
public:
    explicit DashboardContent(std::function<void()> openQuotes);

    void update(const SDL_Event& event) override;
    void render(Renderer& renderer) override;
    void refresh();

private:
    void refreshData();

    std::function<void()> openQuotes;
    std::vector<QuoteRecord> recentQuotes;
    std::vector<QuoteRecord> followUpQuotes;
    int quotesThisMonth = 0;
    int quotesLastMonth = 0;
    int awaitingResponse = 0;
    double quotedThisMonth = 0.0;
    Uint32 dataRefreshedAt = 0;
};
