#include "DashboardContent.h"

#include "MaterialDatabase.h"
#include "QuoteDatabase.h"
#include "Renderer.h"
#include "FontManager.h"
#include "Metrics.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <utility>

namespace
{
    const SDL_Color Ink{ 39, 43, 48, 255 };
    const SDL_Color Secondary{ 112, 119, 127, 255 };
    const SDL_Color Border{ 226, 230, 234, 255 };
    const SDL_Color Green{ 30, 137, 65, 255 };
    const SDL_Color GreenSoft{ 232, 246, 235, 255 };
    const SDL_Color Amber{ 229, 150, 22, 255 };
    const SDL_Color AmberSoft{ 255, 246, 226, 255 };
    const SDL_Color Blue{ 28, 121, 196, 255 };

    std::string recordDate(const QuoteRecord& quote)
    {
        return quote.savedAt.empty() ? quote.invoice.invoiceDate : quote.savedAt;
    }

    std::string statusDate(const QuoteRecord& quote)
    {
        return quote.statusUpdatedAt.empty() ? recordDate(quote) : quote.statusUpdatedAt;
    }

    std::string monthKey(const std::tm& value)
    {
        char buffer[8]{};
        std::snprintf(buffer, sizeof(buffer), "%04d-%02d",
            value.tm_year + 1900, value.tm_mon + 1);
        return buffer;
    }

    bool readDate(const std::string& value, std::tm& parsed)
    {
        int year = 0;
        int month = 0;
        int day = 0;
        if (value.size() < 10 ||
            sscanf_s(value.c_str(), "%4d-%2d-%2d", &year, &month, &day) != 3 ||
            month < 1 || month > 12 || day < 1 || day > 31)
            return false;

        parsed = {};
        parsed.tm_year = year - 1900;
        parsed.tm_mon = month - 1;
        parsed.tm_mday = day;
        parsed.tm_isdst = -1;
        return true;
    }

    int daysSince(const std::string& date, const std::tm& today)
    {
        std::tm parsed{};
        if (!readDate(date, parsed))
            return -1;

        std::tm normalizedToday = today;
        normalizedToday.tm_hour = 12;
        normalizedToday.tm_min = 0;
        normalizedToday.tm_sec = 0;
        parsed.tm_hour = 12;
        parsed.tm_min = 0;
        parsed.tm_sec = 0;

        const std::time_t todayTime = std::mktime(&normalizedToday);
        const std::time_t parsedTime = std::mktime(&parsed);
        if (todayTime == static_cast<std::time_t>(-1) ||
            parsedTime == static_cast<std::time_t>(-1))
            return -1;

        return static_cast<int>(std::difftime(todayTime, parsedTime) / 86400.0);
    }

    double parseAmount(const std::string& value)
    {
        std::string numeric;
        for (char character : value)
        {
            if ((character >= '0' && character <= '9') || character == '.' || character == '-')
                numeric.push_back(character);
        }
        if (numeric.empty())
            return 0.0;
        char* end = nullptr;
        const double amount = std::strtod(numeric.c_str(), &end);
        return end == numeric.c_str() ? 0.0 : amount;
    }

    std::string formatAmount(double amount)
    {
        std::ostringstream formatted;
        formatted << std::fixed << std::setprecision(2) << (std::max)(0.0, amount);
        std::string digits = formatted.str();
        const std::size_t decimal = digits.find('.');
        for (int position = static_cast<int>(decimal) - 3; position > 0; position -= 3)
            digits.insert(static_cast<std::size_t>(position), ",");
        return "R " + digits;
    }

    std::string truncateText(
        TTF_Font* font,
        const std::string& value,
        int maxWidth)
    {
        if (!font || maxWidth <= 0)
            return {};

        int width = 0;
        int height = 0;
        if (TTF_SizeUTF8(font, value.c_str(), &width, &height) == 0 && width <= maxWidth)
            return value;

        std::string shortened = value;
        while (!shortened.empty())
        {
            shortened.pop_back();
            while (!shortened.empty() &&
                (static_cast<unsigned char>(shortened.back()) & 0xC0) == 0x80)
                shortened.pop_back();
            if (!shortened.empty() &&
                (static_cast<unsigned char>(value[shortened.size() - 1]) & 0x80) != 0)
                shortened.pop_back();
            const std::string candidate = shortened + "…";
            if (TTF_SizeUTF8(font, candidate.c_str(), &width, &height) == 0 && width <= maxWidth)
                return candidate;
        }
        return {};
    }

    void drawCheck(Renderer& renderer, int centerX, int centerY)
    {
        renderer.fillCircle(centerX, centerY, 11, GreenSoft);
        renderer.drawLine(centerX - 5, centerY, centerX - 1, centerY + 4, Green);
        renderer.drawLine(centerX - 1, centerY + 4, centerX + 6, centerY - 4, Green);
    }

    void drawMetricIcon(Renderer& renderer, int kind, const SDL_Rect& bounds)
    {
        const int centerX = bounds.x + bounds.w / 2;
        const int centerY = bounds.y + bounds.h / 2;
        const int radius = (std::min)(bounds.w, bounds.h) / 2;
        renderer.fillCircle(centerX, centerY, radius, kind == 1 ? AmberSoft : GreenSoft);
        const SDL_Color color = kind == 1 ? Amber : Green;

        if (kind == 0)
        {
            SDL_Rect document{ centerX - 8, centerY - 10, 16, 20 };
            renderer.drawRect(document, color);
            renderer.drawLine(centerX - 4, centerY - 3, centerX + 4, centerY - 3, color);
            renderer.drawLine(centerX - 4, centerY + 1, centerX + 4, centerY + 1, color);
            renderer.drawLine(centerX - 4, centerY + 5, centerX + 2, centerY + 5, color);
        }
        else if (kind == 1)
        {
            renderer.drawRoundedRing(
                { centerX - 9, centerY - 9, 18, 18 }, color, 9, 2);
            renderer.drawLine(centerX, centerY, centerX, centerY - 5, color);
            renderer.drawLine(centerX, centerY, centerX + 4, centerY + 2, color);
        }
        else
        {
            renderer.fillRoundedRect({ centerX - 10, centerY + 2, 5, 9 }, color, 2);
            renderer.fillRoundedRect({ centerX - 2, centerY - 5, 5, 16 }, color, 2);
            renderer.fillRoundedRect({ centerX + 6, centerY - 10, 5, 21 }, color, 2);
        }
    }

    void drawStatusCard(
        Renderer& renderer,
        const SDL_Rect& card,
        TTF_Font* headingFont,
        TTF_Font* normalFont,
        TTF_Font* smallFont)
    {
        renderer.fillRoundedRect(card, { 255, 255, 255, 255 }, 10);
        renderer.drawRoundedRect(card, Border, 10);
        renderer.drawText("System Status", card.x + 16, card.y + 12, headingFont, Ink);

        const int contentY = card.y + 67;
        const int summaryWidth = (std::max)(140, static_cast<int>(card.w * 0.22f));
        const int separatorX = card.x + card.w - summaryWidth;
        renderer.drawLine(separatorX, card.y + 28, separatorX, card.y + card.h - 22, Border);

        const int statusWidth = (separatorX - card.x - 36) / 3;
        const char* statuses[] = { "Materials Loaded", "Pricing Loaded", "Production Pricing Loaded" };
        for (int i = 0; i < 3; ++i)
        {
            const int itemX = card.x + 18 + statusWidth * i;
            drawCheck(renderer, itemX + 10, contentY + 1);
            renderer.drawText(
                std::string("[OK] ") + statuses[i],
                itemX + 28,
                contentY - 8,
                smallFont,
                Secondary);
        }

        auto materials = MaterialDatabase::getBaseMaterials();
        std::size_t variants = 0;
        for (const auto& id : materials)
            variants += MaterialDatabase::get(id).variants.size();

        const int metricsX = separatorX + 12;
        const int metricWidth = (card.x + card.w - metricsX - 16) / 2;
        renderer.drawText("Materials", metricsX, card.y + 42, smallFont, Secondary);
        renderer.drawText(std::to_string(materials.size()), metricsX, card.y + 67, headingFont, Ink);
        renderer.drawText("Variants", metricsX + metricWidth, card.y + 42, smallFont, Secondary);
        renderer.drawText(std::to_string(variants), metricsX + metricWidth, card.y + 67, headingFont, Ink);
        (void)normalFont;
    }

    SDL_Color quoteStatusColor(const std::string& status)
    {
        if (status == "Accepted" || status == "Approved")
            return { 225, 247, 234, 255 };
        if (status == "Sent")
            return { 226, 242, 253, 255 };
        if (status == "Declined" || status == "Expired")
            return { 253, 232, 232, 255 };
        return { 239, 241, 243, 255 };
    }

    SDL_Color quoteStatusTextColor(const std::string& status)
    {
        if (status == "Accepted" || status == "Approved")
            return Green;
        if (status == "Sent")
            return { 24, 111, 183, 255 };
        if (status == "Declined" || status == "Expired")
            return { 177, 58, 58, 255 };
        return Ink;
    }

    std::string displayStatus(const std::string& status)
    {
        return status == "Accepted" ? "Approved" : status;
    }
}

DashboardContent::DashboardContent(std::function<void()> callback)
    : openQuotes(std::move(callback))
{
}

void DashboardContent::refresh()
{
    dataRefreshedAt = 0;
}

void DashboardContent::update(const SDL_Event& event)
{
    if (!visible || event.type != SDL_MOUSEBUTTONUP || event.button.button != SDL_BUTTON_LEFT)
        return;

    SDL_Point point{ event.button.x, event.button.y };
    const int leftWidth = static_cast<int>((bounds.w - Metrics::SpaceL) * 0.41f);
    const int lowerY = bounds.y + 156;
    const int followUpY = lowerY + 301;
    const SDL_Rect recentCardArea{
        bounds.x + leftWidth + Metrics::SpaceL,
        lowerY,
        bounds.w - leftWidth - Metrics::SpaceL,
        bounds.h - 156
    };
    const SDL_Rect followUpArea{
        bounds.x,
        followUpY,
        leftWidth,
        (std::max)(0, bounds.h - (followUpY - bounds.y))
    };
    if ((SDL_PointInRect(&point, &recentCardArea) ||
        SDL_PointInRect(&point, &followUpArea)) && openQuotes)
        openQuotes();
}

void DashboardContent::refreshData()
{
    std::tm today{};
    const std::time_t now = std::time(nullptr);
    localtime_s(&today, &now);
    const std::string currentMonth = monthKey(today);
    std::tm previousMonthDate = today;
    previousMonthDate.tm_mday = 1;
    previousMonthDate.tm_mon -= 1;
    std::mktime(&previousMonthDate);
    const std::string previousMonth = monthKey(previousMonthDate);

    const auto& records = QuoteDatabase::getAll();
    std::vector<QuoteRecord> sorted = records;
    std::sort(sorted.begin(), sorted.end(), [](const QuoteRecord& left, const QuoteRecord& right)
    {
        return recordDate(left) > recordDate(right);
    });

    quotesThisMonth = 0;
    quotesLastMonth = 0;
    awaitingResponse = 0;
    quotedThisMonth = 0.0;
    followUpQuotes.clear();
    recentQuotes.clear();

    for (const auto& quote : sorted)
    {
        const std::string date = recordDate(quote);
        if (date.size() >= 7 && date.substr(0, 7) == currentMonth)
        {
            ++quotesThisMonth;
            quotedThisMonth += parseAmount(quote.invoice.total);
        }
        else if (date.size() >= 7 && date.substr(0, 7) == previousMonth)
        {
            ++quotesLastMonth;
        }

        if (quote.status == "Sent")
        {
            ++awaitingResponse;
            if (daysSince(statusDate(quote), today) >= 5)
                followUpQuotes.push_back(quote);
        }

        if (recentQuotes.size() < 3)
            recentQuotes.push_back(quote);
    }

    if (followUpQuotes.size() > 2)
    {
        std::sort(followUpQuotes.begin(), followUpQuotes.end(), [](const QuoteRecord& left, const QuoteRecord& right)
        {
            return statusDate(left) < statusDate(right);
        });
        followUpQuotes.resize(2);
    }

    dataRefreshedAt = SDL_GetTicks();
}

void DashboardContent::render(Renderer& renderer)
{
    if (!visible)
        return;

    const Uint32 now = SDL_GetTicks();
    if (dataRefreshedAt == 0 || now - dataRefreshedAt >= 1000)
        refreshData();

    FontManager& fonts = renderer.getFontManager();
    TTF_Font* headingFont = fonts.getHeadingFont();
    TTF_Font* normalFont = fonts.getNormalFont();
    TTF_Font* smallFont = fonts.getSmallFont();
    std::tm today{};
    const std::time_t currentTime = std::time(nullptr);
    localtime_s(&today, &currentTime);

    constexpr int gap = Metrics::SpaceL;
    const SDL_Rect statusCard{ bounds.x, bounds.y, bounds.w, 142 };
    drawStatusCard(renderer, statusCard, headingFont, normalFont, smallFont);

    const int lowerY = bounds.y + statusCard.h + gap;
    const int lowerHeight = (std::max)(0, bounds.h - statusCard.h - gap);
    const int columnGap = Metrics::SpaceL;
    const int leftWidth = static_cast<int>((bounds.w - columnGap) * 0.41f);
    const SDL_Rect glanceCard{ bounds.x, lowerY, leftWidth, lowerHeight };
    const SDL_Rect recentCard{ bounds.x + leftWidth + columnGap, lowerY,
        bounds.w - leftWidth - columnGap, lowerHeight };

    renderer.fillRoundedRect(glanceCard, { 255, 255, 255, 255 }, 10);
    renderer.drawRoundedRect(glanceCard, Border, 10);
    renderer.drawText("At a glance", glanceCard.x + 16, glanceCard.y + 14, headingFont, Ink);

    const int metricX = glanceCard.x + 16;
    const int metricWidth = glanceCard.w - 32;
    const int metricHeight = 76;
    const int metricGap = 9;
    const int metricY = glanceCard.y + 55;
    const int iconSize = 48;
    const int textX = metricX + 16 + iconSize + 13;

    const SDL_Rect metricRects[] = {
        { metricX, metricY, metricWidth, metricHeight },
        { metricX, metricY + metricHeight + metricGap, metricWidth, metricHeight },
        { metricX, metricY + 2 * (metricHeight + metricGap), metricWidth, metricHeight }
    };
    for (const auto& rect : metricRects)
    {
        renderer.fillRoundedRect(rect, { 255, 255, 255, 255 }, 9);
        renderer.drawRoundedRect(rect, Border, 9);
    }

    drawMetricIcon(renderer, 0, { metricX + 16, metricY + 14, iconSize, iconSize });
    renderer.drawText("Quotes this month", textX, metricY + 14, smallFont, Ink);
    renderer.drawText(std::to_string(quotesThisMonth), textX, metricY + 36, headingFont, Ink);
    if (metricWidth > 340)
    {
        const int trendX = metricX + metricWidth - 112;
        std::string trendText = "0%";
        SDL_Color trendColor = Secondary;
        bool trendDown = false;
        if (quotesLastMonth == 0 && quotesThisMonth > 0)
        {
            trendText = "New";
            trendColor = Green;
        }
        else if (quotesLastMonth > 0)
        {
            const int percentage = static_cast<int>(std::round(
                100.0 * (quotesThisMonth - quotesLastMonth) / quotesLastMonth));
            trendText = (percentage > 0 ? "+" : "") + std::to_string(percentage) + "%";
            trendColor = percentage >= 0 ? Green : SDL_Color{ 177, 58, 58, 255 };
            trendDown = percentage < 0;
        }
        const int arrowTipY = metricY + (trendDown ? 32 : 24);
        renderer.drawLine(trendX, metricY + (trendDown ? 24 : 30), trendX + 6, arrowTipY, trendColor);
        renderer.drawLine(trendX + 6, arrowTipY, trendX + 6, arrowTipY - (trendDown ? 4 : -4), trendColor);
        renderer.drawLine(trendX + 6, arrowTipY, trendX + (trendDown ? 2 : 2),
            arrowTipY - (trendDown ? 4 : -4), trendColor);
        renderer.drawText(trendText, trendX + 12, metricY + 20, smallFont, trendColor);
        renderer.drawText("vs last month", trendX, metricY + 43, smallFont, Secondary);
    }

    const int secondY = metricY + metricHeight + metricGap;
    drawMetricIcon(renderer, 1, { metricX + 16, secondY + 14, iconSize, iconSize });
    renderer.drawText("Awaiting response", textX, secondY + 14, smallFont, Ink);
    renderer.drawText(std::to_string(awaitingResponse), textX, secondY + 36, headingFont, Ink);

    const int thirdY = metricY + 2 * (metricHeight + metricGap);
    drawMetricIcon(renderer, 2, { metricX + 16, thirdY + 14, iconSize, iconSize });
    renderer.drawText("Quoted this month", textX, thirdY + 11, smallFont, Ink);
    renderer.drawText(formatAmount(quotedThisMonth), textX, thirdY + 30, headingFont, Ink);
    renderer.drawText("Across " + std::to_string(quotesThisMonth) + " quotes",
        textX, thirdY + 55, smallFont, Secondary);

    const int followY = thirdY + metricHeight + 16;
    renderer.drawText("Follow-up needed", metricX + 2, followY, normalFont, Ink);
    renderer.drawLine(metricX, followY + 28, metricX + metricWidth, followY + 28, Border);

    if (followUpQuotes.empty())
    {
        renderer.drawText("No quotes need a follow-up yet.", metricX + 2, followY + 42, smallFont, Secondary);
    }
    else
    {
        for (std::size_t i = 0; i < followUpQuotes.size(); ++i)
        {
            const int rowY = followY + 39 + static_cast<int>(i) * 53;
            const auto& quote = followUpQuotes[i];
            const std::string customer = quote.invoice.customer.empty()
                ? "Unnamed customer" : quote.invoice.customer;
            const int reviewWidth = 48;
            const std::string line = customer + " · " + quote.invoice.jobref;
            renderer.drawText(truncateText(smallFont, line, metricWidth - reviewWidth - 14),
                metricX + 2, rowY, smallFont, Ink);
            renderer.drawText("Review", metricX + metricWidth - reviewWidth, rowY,
                smallFont, Blue);

            const int age = daysSince(statusDate(quote), today);
            const std::string dateVerb = quote.statusUpdatedAt.empty() ? "Saved" : "Sent";
            const std::string ageText = age == 1 ? dateVerb + " 1 day ago"
                : dateVerb + " " + std::to_string((std::max)(age, 0)) + " days ago";
            renderer.drawText(ageText, metricX + 2, rowY + 19, smallFont, Secondary);
            if (i + 1 < followUpQuotes.size())
                renderer.drawLine(metricX, rowY + 43, metricX + metricWidth, rowY + 43, Border);
        }
    }

    renderer.fillRoundedRect(recentCard, { 255, 255, 255, 255 }, 10);
    renderer.drawRoundedRect(recentCard, Border, 10);
    renderer.drawText("Recent quotes", recentCard.x + 16, recentCard.y + 14, headingFont, Ink);
    renderer.drawText("Your most recent quote activity.", recentCard.x + 16,
        recentCard.y + 43, smallFont, Secondary);
    const std::string viewAllText = "View all quotes";
    int viewAllWidth = 0;
    int viewAllHeight = 0;
    TTF_SizeUTF8(smallFont, viewAllText.c_str(), &viewAllWidth, &viewAllHeight);
    const int viewAllY = recentCard.y + 20;
    const int chevronX = recentCard.x + recentCard.w - 16;
    renderer.drawText(viewAllText, chevronX - viewAllWidth - 8,
        viewAllY, smallFont, Green);
    renderer.drawLine(chevronX - 3, viewAllY + 1, chevronX, viewAllY + 5, Green);
    renderer.drawLine(chevronX, viewAllY + 5, chevronX - 3, viewAllY + 9, Green);

    const int tableX = recentCard.x + 14;
    const int tableY = recentCard.y + 81;
    const int tableWidth = recentCard.w - 28;
    const int tableHeaderHeight = 39;
    const int rowHeight = 58;
    renderer.fillRect({ tableX, tableY, tableWidth, tableHeaderHeight }, { 247, 248, 249, 255 });
    renderer.drawRect({ tableX, tableY, tableWidth, tableHeaderHeight }, Border);

    const int referenceWidth = static_cast<int>(tableWidth * 0.29f);
    const int customerWidth = static_cast<int>(tableWidth * 0.34f);
    const int totalWidth = static_cast<int>(tableWidth * 0.20f);
    const int statusWidth = tableWidth - referenceWidth - customerWidth - totalWidth;
    const int referenceX = tableX + 12;
    const int customerX = tableX + referenceWidth + 8;
    const int totalX = tableX + referenceWidth + customerWidth + 6;
    const int statusX = tableX + referenceWidth + customerWidth + totalWidth;
    renderer.drawText("Reference", referenceX, tableY + 10, smallFont, Ink);
    renderer.drawText("Customer", customerX, tableY + 10, smallFont, Ink);
    renderer.drawText("Total", totalX, tableY + 10, smallFont, Ink);
    renderer.drawText("Status", statusX + 6, tableY + 10, smallFont, Ink);

    if (recentQuotes.empty())
    {
        renderer.drawText("No quotes saved yet.", tableX + 12, tableY + tableHeaderHeight + 16,
            smallFont, Secondary);
    }
    else
    {
        for (std::size_t i = 0; i < recentQuotes.size(); ++i)
        {
            const int rowY = tableY + tableHeaderHeight + static_cast<int>(i) * rowHeight;
            const auto& quote = recentQuotes[i];
            renderer.fillRect({ tableX, rowY, tableWidth, rowHeight }, { 255, 255, 255, 255 });
            renderer.drawRect({ tableX, rowY, tableWidth, rowHeight }, Border);

            renderer.drawText(truncateText(smallFont,
                quote.invoice.jobref.empty() ? "(no reference)" : quote.invoice.jobref,
                referenceWidth - 16), referenceX, rowY + 20, smallFont, Ink);
            renderer.drawText(truncateText(smallFont,
                quote.invoice.customer.empty() ? "Unnamed customer" : quote.invoice.customer,
                customerWidth - 14), customerX, rowY + 20, smallFont, Ink);
            renderer.drawText(truncateText(smallFont,
                quote.invoice.total.empty() ? "—" : formatAmount(parseAmount(quote.invoice.total)),
                totalWidth - 8), totalX, rowY + 20, smallFont, Ink);

            const std::string status = displayStatus(quote.status);
            int statusTextWidth = 0;
            int statusTextHeight = 0;
            TTF_SizeUTF8(smallFont, status.c_str(), &statusTextWidth, &statusTextHeight);
            const int chipWidth = (std::min)(statusWidth - 12,
                (std::max)(58, statusTextWidth + 20));
            const int chipHeight = 30;
            const SDL_Rect chip{ statusX + 5, rowY + (rowHeight - chipHeight) / 2,
                chipWidth, chipHeight };
            renderer.fillRoundedRect(chip, quoteStatusColor(quote.status), 15);
            renderer.drawText(status, chip.x + (chip.w - statusTextWidth) / 2,
                chip.y + 7, smallFont, quoteStatusTextColor(quote.status));
        }
    }
}
