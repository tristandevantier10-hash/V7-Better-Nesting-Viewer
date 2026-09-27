#include "SheetPreviewPanel.h"
#include "NestingRender.h"
#include "Renderer.h"
#include "iostream"
#include <algorithm>
#include <cmath>

SheetPreviewPanel::SheetPreviewPanel()
{
    viewMode = ViewMode::FitWidth;
}

void SheetPreviewPanel::setSheets(
    const std::vector<Sheet>& newSheets)
{
    sheets = newSheets;
}

void SheetPreviewPanel::setViewMode(ViewMode mode)
{
    viewMode = mode;

    // Whenever the view changes, reset the scroll position.
    scrollY = 0;
}

void SheetPreviewPanel::render(Renderer& renderer)
{
    if (!visible)
        return;

    performLayout();

    if (viewMode == ViewMode::FitWidth)
        renderFitWidth(renderer);
    else
        renderFitAll(renderer);
}

void SheetPreviewPanel::update(const SDL_Event& e)
{
    if (e.type == SDL_MOUSEBUTTONDOWN &&
        e.button.button == SDL_BUTTON_LEFT)
    {
        SDL_Point mouse =
        {
            e.button.x,
            e.button.y
        };

        if (SDL_PointInRect(&mouse, &fitWidthButton))
        {
            setViewMode(ViewMode::FitWidth);
            return;
        }

        if (SDL_PointInRect(&mouse, &fitAllButton))
        {
            setViewMode(ViewMode::FitAll);
            return;
        }

        // <<< ADD THE ZOOM BUTTONS HERE >>>

        if (SDL_PointInRect(&mouse, &zoomInButton))
        {
            zoom *= 1.1f;

            if (zoom > 8.0f)
                zoom = 8.0f;

            return;
        }

        if (SDL_PointInRect(&mouse, &zoomOutButton))
        {
            zoom /= 1.1f;

            if (zoom < 0.2f)
                zoom = 0.2f;

            return;
        }
    }

    if (e.type == SDL_MOUSEWHEEL)
    {

        const Uint8* keys = SDL_GetKeyboardState(nullptr);

        if (keys[SDL_SCANCODE_LCTRL] || keys[SDL_SCANCODE_RCTRL])
        {
            if (e.wheel.y > 0)
                zoom *= 1.1f;
            else
                zoom /= 1.1f;

            zoom = std::clamp(zoom, 0.2f, 8.0f);

            return;
        }

        int mx, my;
        SDL_GetMouseState(&mx, &my);

        SDL_Point mouse{ mx, my };

        if (!SDL_PointInRect(&mouse, &bounds))
            return;

        scrollY -= e.wheel.y * 40;

        float contentHeight =
            calculateContentHeight();

        maxScrollY =
            contentHeight - getHeight();

        if (maxScrollY < 0)
            maxScrollY = 0;

        if (scrollY < 0)
            scrollY = 0;

        if (scrollY > maxScrollY)
            scrollY = maxScrollY;
    }
}

float SheetPreviewPanel::calculateScale() const
{
    if (viewMode == ViewMode::FitWidth)
    {
        if (sheets.empty())
            return 1.0f;

        double maxWidth = 0;

        for (const auto& sheet : sheets)
        {
            if (sheet.width > maxWidth)
                maxWidth = sheet.width;
        }

        float availableWidth =
            getWidth() - 40;

        float scaleX =
            availableWidth / maxWidth;

        return scaleX * zoom;
    }
    else
    {
        if (sheets.empty())
            return 1.0f;

        double maxWidth = 0;
        double totalHeight = 20; // top padding

        for (const auto& sheet : sheets)
        {
            if (sheet.width > maxWidth)
                maxWidth = sheet.width;

            totalHeight += 30;          // title
            totalHeight += sheet.height;
            totalHeight += 80;          // spacing
        }

        totalHeight += 20;              // bottom padding

        float availableWidth =
            getWidth() - 40;

        float availableHeight =
            getHeight() - 40;

        float scaleX =
            availableWidth / maxWidth;

        float scaleY =
            availableHeight / totalHeight;

        return std::min(scaleX, scaleY) * zoom;
    }
}

float SheetPreviewPanel::calculateContentHeight() const
{
    float scale = calculateScale();

    float height = 20; // top padding


    for (const auto& sheet : sheets)
    {
        height += 30; // title

        height +=
            (sheet.height * scale);

        height += 80; // spacing
    }


    return height + 20;
}

void SheetPreviewPanel::renderFitWidth(Renderer& renderer)
{
    SDL_Renderer* sdl =
        renderer.getSDLRenderer();

    SDL_Rect panel =
    {
        getX(),
        getY(),
        getWidth(),
        getHeight()
    };

    renderer.fillRect(
        panel,
        SDL_Color{ 25,25,30,255 });

    // ----------------------------------------------------
    // Toolbar
    // ----------------------------------------------------

    drawToolbar(renderer, panel);

    renderer.pushClip(panel);

    float scale = calculateScale();

    int x = 0;
    int y = panel.y + 55 - scrollY;

    int sheetNumber = 1;

    for (const auto& sheet : sheets)
    {
        float drawnWidth =
            sheet.width * scale;

        float drawnHeight =
            sheet.height * scale;

        x =
            panel.x +
            (panel.w - static_cast<int>(drawnWidth)) / 2;

        std::string title =
            "Sheet " + std::to_string(sheetNumber) +
            " (" +
            std::to_string((int)sheet.width) +
            " x " +
            std::to_string((int)sheet.height) +
            " mm)";

        int textWidth =
            renderer.getTextWidth(title);

        int titleX =
            x +
            (static_cast<int>(drawnWidth) - textWidth) / 2;

        renderer.drawText(
            title,
            titleX,
            y);

        y += 30;

        NestingRenderer::drawSheet(
            renderer,
            sheet,
            x,
            y,
            scale);

        y += static_cast<int>(drawnHeight);

        y += 60;

        sheetNumber++;
    }

    renderer.popClip();
}

void SheetPreviewPanel::renderFitAll(Renderer& renderer)
{
    SDL_Rect panel =
    {
        getX(),
        getY(),
        getWidth(),
        getHeight()
    };

    renderer.fillRect(
        panel,
        SDL_Color{ 25,25,30,255 });

    // ----------------------------------------------------
    // Toolbar
    // ----------------------------------------------------

    drawToolbar(renderer, panel);

    renderer.pushClip(panel);

    int sheetCount = static_cast<int>(sheets.size());

    if (sheetCount == 0)
    {
        renderer.popClip();
        return;
    }

    int columns =
        static_cast<int>(std::ceil(std::sqrt(sheetCount)));

    int rows =
        static_cast<int>(std::ceil(
            sheetCount /
            static_cast<float>(columns)));

    const int margin = 20;
    const int toolbarHeight = 50;

    float cellWidth =
        (panel.w - (columns + 1) * margin) /
        static_cast<float>(columns);

    float cellHeight =
        (panel.h - toolbarHeight - (rows + 1) * margin) /
        static_cast<float>(rows);

    double maxWidth = 0;
    double maxHeight = 0;

    for (const auto& sheet : sheets)
    {
        maxWidth = std::max(maxWidth, sheet.width);
        maxHeight = std::max(maxHeight, sheet.height);
    }

    float scaleX =
        cellWidth /
        static_cast<float>(maxWidth);

    float scaleY =
        (cellHeight - 20) /
        static_cast<float>(maxHeight);

    float scale =
        std::min(scaleX, scaleY) * zoom;

    int sheetNumber = 1;

    for (const auto& sheet : sheets)
    {
        int column =
            (sheetNumber - 1) % columns;

        int row =
            (sheetNumber - 1) / columns;

        int drawnWidth =
            static_cast<int>(sheet.width * scale);

        int drawnHeight =
            static_cast<int>(sheet.height * scale);

        int cellX =
            panel.x + margin +
            static_cast<int>(
                column * (cellWidth + margin));

        int cellY =
            panel.y + toolbarHeight + margin +
            static_cast<int>(
                row * (cellHeight + margin));

        int x =
            cellX +
            (static_cast<int>(cellWidth) - drawnWidth) / 2;

        int y =
            cellY + 5;

        std::string title =
            "Sheet " + std::to_string(sheetNumber) +
            " (" +
            std::to_string((int)sheet.width) +
            " x " +
            std::to_string((int)sheet.height) +
            " mm)";

        int textWidth =
            renderer.getTextWidth(title);

        renderer.drawText(
            title,
            x + (drawnWidth - textWidth) / 2,
            y);

        NestingRenderer::drawSheet(
            renderer,
            sheet,
            x,
            y + 20,
            scale);

        sheetNumber++;
    }

    renderer.popClip();
}

void SheetPreviewPanel::drawButton(
    Renderer& renderer,
    const SDL_Rect& rect,
    const std::string& text,
    bool active)
{
    renderer.fillRect(
        rect,
        active
        ? SDL_Color{ 90,140,220,255 }
    : SDL_Color{ 60,60,70,255 });

    int textWidth =
        renderer.getTextWidth(text);

    int textX =
        rect.x +
        (rect.w - textWidth) / 2;

    int textY =
        rect.y + 8;

    renderer.drawText(
        text,
        textX,
        textY);
}

void SheetPreviewPanel::drawToolbar(
    Renderer& renderer,
    const SDL_Rect& panel)
{
    int x = panel.x + 10;
    int y = panel.y + 10;

    // ---------------------------
    // Fit Width
    // ---------------------------

    fitWidthButton =
    {
        x,
        y,
        100,
        30
    };

    drawButton(
        renderer,
        fitWidthButton,
        "Fit Width",
        viewMode == ViewMode::FitWidth);

    x += 110;

    // ---------------------------
    // Fit All
    // ---------------------------

    fitAllButton =
    {
        x,
        y,
        100,
        30
    };

    drawButton(
        renderer,
        fitAllButton,
        "Fit All",
        viewMode == ViewMode::FitAll);

    x += 120;

    // ---------------------------
    // Zoom In
    // ---------------------------

    zoomInButton =
    {
        x,
        y,
        30,
        30
    };

    drawButton(
        renderer,
        zoomInButton,
        "+",
        false);

    x += 40;

    // ---------------------------
    // Zoom Out
    // ---------------------------

    zoomOutButton =
    {
        x,
        y,
        30,
        30
    };

    drawButton(
        renderer,
        zoomOutButton,
        "-",
        false);
}