#pragma once

#include "NestingStyle.h"
#include "NestingEngine.h"
#include <cmath>
#include <algorithm>

class NestingPreviewPanel : public UIElement
{
public:
    void setSheets(const std::vector<Sheet>& value)
    {
        sheets = value;
        scroll = 0;
        if (!explicitItems) inferItems();
    }
    void setItems(const std::vector<Rect>& value)
    {
        geometryParts.clear();
        items = value;
        explicitItems = !value.empty();
        if (!explicitItems) inferItems();
    }
    void setGeometryParts(const std::vector<NestingGeometry::Part>& value)
    {
        std::vector<Rect> sizes;
        for (const auto& part:value)
        {
            const auto box=NestingGeometry::bounds(part.outline);
            sizes.push_back({box.right-box.left,box.bottom-box.top,part.quantity,part.id});
        }
        setItems(sizes);
        geometryParts=value;
    }
    void resetView() { fitAll = true; zoom = 1.0; scroll = 0; }
    std::string status;

    void update(const SDL_Event& event) override
    {
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
        {
            SDL_Point point{ event.button.x, event.button.y };
            if (SDL_PointInRect(&point, &allButton)) { fitAll = true; zoom = 1; scroll = 0; }
            else if (SDL_PointInRect(&point, &widthButton)) { fitAll = false; zoom = 1; scroll = 0; }
            else if (SDL_PointInRect(&point, &minusButton)) zoom = std::max(0.5, zoom - 0.1);
            else if (SDL_PointInRect(&point, &plusButton)) zoom = std::min(3.0, zoom + 0.1);
        }
        if (event.type == SDL_MOUSEWHEEL)
        {
            SDL_Point mouse{};
            SDL_GetMouseState(&mouse.x, &mouse.y);
            if (SDL_PointInRect(&mouse, &canvas)) scroll = std::clamp(scroll - event.wheel.y * 40, 0, maxScroll);
        }
    }

    void render(Renderer& renderer) override
    {
        using namespace NestingStyle;
        card(renderer, bounds);
        const int pad = 15;
        const bool compact = bounds.w < 560;
        const int toolbar = compact ? 72 : 54;
        const int footer = compact ? (items.size() > 2 ? 60 : 34) : 44;
        text(renderer, "Nesting layout", bounds.x + pad, bounds.y + 14, LabelStyle::Normal);
        const int buttonWidth = compact ? 68 : 78;
        allButton = { compact ? bounds.x + 12 : bounds.x + bounds.w - 325,
            bounds.y + (compact ? 35 : 12), buttonWidth, 30 };
        widthButton = { allButton.x + buttonWidth, allButton.y, buttonWidth, 30 };
        minusButton = { compact ? bounds.x + bounds.w - 124 : widthButton.x + 96,
            allButton.y, 28, 30 };
        plusButton = { bounds.x + bounds.w - 40, allButton.y, 28, 30 };
        auto control = [&](const SDL_Rect& rect, const std::string& label, bool selected)
        {
            renderer.fillRoundedRect(rect, selected ? green : white, 5);
            renderer.drawRoundedRect(rect, selected ? green : border, 5);
            int w = 0, h = 0;
            auto* font = renderer.getFontManager().getSmallFont();
            TTF_SizeUTF8(font, label.c_str(), &w, &h);
            renderer.drawText(label, rect.x + (rect.w - w) / 2, rect.y + (rect.h - h) / 2,
                font, selected ? white : ink);
        };
        control(allButton, "Fit all", fitAll);
        control(widthButton, "Fit width", !fitAll);
        control(minusButton, "−", false);
        control({ minusButton.x + 28, minusButton.y, plusButton.x - minusButton.x - 28, 30 },
            std::to_string(static_cast<int>(std::round(zoom * 100))) + "%", false);
        control(plusButton, "+", false);

        canvas = { bounds.x + 1, bounds.y + toolbar, bounds.w - 2, std::max(1, bounds.h - toolbar - footer) };
        renderer.pushClip(canvas);
        renderer.fillRect(canvas, { 251, 252, 253, 255 });
        for (int x = canvas.x; x < canvas.x + canvas.w; x += 18)
            renderer.drawLine(x, canvas.y, x, canvas.y + canvas.h, { 239, 243, 247, 255 });
        for (int y = canvas.y; y < canvas.y + canvas.h; y += 18)
            renderer.drawLine(canvas.x, y, canvas.x + canvas.w, y, { 239, 243, 247, 255 });
        const int columns = fitAll && canvas.w >= 450 ? 2 : 1;
        const int gap = compact ? 8 : 14;
        const int cellWidth = (canvas.w - gap * (columns + 1)) / columns;
        const int paperInset = compact ? 8 : 12;
        const int paperTop = compact ? 44 : 50;
        const int cardExtra = compact ? 52 : 66;
        double largestRatio = 0.0;
        for (const auto& sheet : sheets)
            if (sheet.width > 0) largestRatio = std::max(largestRatio, sheet.height / sheet.width);
        // In the small quote card, fit a complete sheet below its controls.
        const double naturalHeight = (cellWidth - paperInset * 2) * largestRatio;
        const double paperHeight = compact && fitAll
            ? std::min(naturalHeight, static_cast<double>(std::max(12, canvas.h - gap * 2 - cardExtra)))
            : naturalHeight;
        const int cellHeight = static_cast<int>(paperHeight * zoom) + cardExtra;
        const int rowCount = (static_cast<int>(sheets.size()) + columns - 1) / columns;
        maxScroll = std::max(0, rowCount * (cellHeight + gap) + gap - canvas.h);
        scroll = std::clamp(scroll, 0, maxScroll);
        for (size_t index = 0; index < sheets.size(); ++index)
        {
            const auto& sheet = sheets[index];
            SDL_Rect cell{ canvas.x + gap + static_cast<int>(index % columns) * (cellWidth + gap),
                canvas.y + gap + static_cast<int>(index / columns) * (cellHeight + gap) - scroll,
                cellWidth, cellHeight };
            if (cell.y + cell.h < canvas.y || cell.y > canvas.y + canvas.h) continue;
            card(renderer, cell);
            renderer.pushClip(cell);
            std::string title = "Sheet " + std::string(index < 9 ? "0" : "") + std::to_string(index + 1);
            text(renderer, title, cell.x + 12, cell.y + 10);
            std::string count = std::to_string(sheet.placed.size()) + (sheet.placed.size()==1 ? " piece" : " pieces");
            int tw = 0, th = 0;
            TTF_SizeUTF8(renderer.getFontManager().getSmallFont(), count.c_str(), &tw, &th);
            text(renderer, count, cell.x + cell.w - tw - 12, cell.y + 10);
            text(renderer, number(sheet.width) + " × " + number(sheet.height) + " mm",
                cell.x + 12, cell.y + 29, LabelStyle::Small, muted);
            const double scale = std::min((cellWidth - paperInset * 2) / sheet.width,
                paperHeight / sheet.height) * zoom;
            SDL_Rect paper{ compact ? cell.x + (cell.w - static_cast<int>(sheet.width * scale)) / 2
                : cell.x + paperInset, cell.y + paperTop,
                static_cast<int>(sheet.width * scale), static_cast<int>(sheet.height * scale) };
            renderer.fillRect(paper, { 220, 222, 223, 255 });
            for (const auto& part : sheet.placed)
            {
                size_t type = 0;
                for (size_t i = 0; i < items.size(); ++i)
                {
                    const auto& item = items[i];
                    if (item.partId != 0 ? item.partId == part.partId :
                        ((std::abs(part.width - item.width) < 1e-6 && std::abs(part.height - item.height) < 1e-6) ||
                        (std::abs(part.height - item.width) < 1e-6 && std::abs(part.width - item.height) < 1e-6)))
                    { type = i; break; }
                }
                const int left = paper.x + static_cast<int>(std::round(part.x * scale));
                const int top = paper.y + static_cast<int>(std::round(part.y * scale));
                SDL_Rect piece{ left, top,
                    paper.x + static_cast<int>(std::round((part.x + part.width) * scale)) - left,
                    paper.y + static_cast<int>(std::round((part.y + part.height) * scale)) - top };
                if (part.contour)
                {
                    // Fill all rings together using even/odd crossings. Never
                    // paint holes with paper colour: that would erase nested parts.
                    renderer.pushClip(paper);
                    for (int y=std::max(paper.y,piece.y);y<std::min(paper.y+paper.h,piece.y+piece.h+1);++y)
                    {
                        const auto intersections=NestingGeometry::scanline(*part.contour,(y+0.5-paper.y)/scale);
                        for (size_t edge=0;edge+1<intersections.size();edge+=2)
                        {
                            const int left=static_cast<int>(std::ceil(paper.x+intersections[edge]*scale-0.5));
                            const int right=static_cast<int>(std::ceil(paper.x+intersections[edge+1]*scale-0.5))-1;
                            if (left<=right) renderer.drawLine(left,y,right,y,colours[type%colours.size()]);
                        }
                    }
                    for (const auto* boundary:NestingGeometry::boundaryRings(*part.contour))
                     for (size_t edge=0;edge<boundary->vertices.size();++edge)
                    {
                        const auto a=boundary->vertices[edge],b=boundary->vertices[(edge+1)%boundary->vertices.size()];
                        renderer.drawLine(paper.x+static_cast<int>(std::round(a.x*scale)),paper.y+static_cast<int>(std::round(a.y*scale)),
                            paper.x+static_cast<int>(std::round(b.x*scale)),paper.y+static_cast<int>(std::round(b.y*scale)),{239,248,243,255});
                    }
                    renderer.popClip();
                }
                else
                {
                    renderer.fillRect(piece, colours[type % colours.size()]);
                    renderer.drawRect(piece, { 239, 248, 243, 255 });
                }
            }
            renderer.drawRect(paper, { 113, 124, 124, 255 });
            renderer.popClip();
        }
        if (sheets.empty())
        {
            renderer.pushClip({ canvas.x + 20, canvas.y + 24, canvas.w - 40, 100 });
            text(renderer, status, canvas.x + 20, canvas.y + 26, LabelStyle::Small, muted);
            renderer.popClip();
        }
        renderer.popClip();
        const int legendY = bounds.y + bounds.h - footer;
        renderer.drawLine(bounds.x + 1, legendY, bounds.x + bounds.w - 1, legendY, border);
        renderer.pushClip({ bounds.x + 10, legendY + 1, bounds.w - 20, footer - 2 });
        int x = bounds.x + 16;
        int legendRow = 0;
        for (size_t i = 0; i < items.size(); ++i)
        {
            std::string label = number(items[i].width) + " × " + number(items[i].height) + " mm";
            const auto definition=std::find_if(geometryParts.begin(),geometryParts.end(),[&](const NestingGeometry::Part& part)
            { return part.id==items[i].partId; });
            if (definition!=geometryParts.end() && !definition->name.empty() && definition->name!="Rectangle")
                label=definition->name+" · "+label;
            int width = 0, height = 0;
            TTF_SizeUTF8(renderer.getFontManager().getSmallFont(), label.c_str(), &width, &height);
            const int right = bounds.x + bounds.w - (compact ? 12 : 150);
            if (x + width + 32 > right)
            {
                if (!compact || legendRow >= 1) break;
                x = bounds.x + 16;
                ++legendRow;
            }
            const int offset = legendRow * 26;
            renderer.fillRoundedRect({ x, legendY + 10 + offset, 18, 18 }, colours[i % colours.size()], 4);
            text(renderer, label, x + 27, legendY + 11 + offset, LabelStyle::Small, muted);
            x += width + 54;
        }
        if (!sheets.empty() && !compact)
            text(renderer, "Best layout found", bounds.x + bounds.w - 133, legendY + 14,
                LabelStyle::Small, muted);
        renderer.popClip();
    }
private:
    void inferItems()
    {
        items.clear();
        for (const auto& sheet : sheets)
            for (const auto& part : sheet.placed)
            {
                const double width = part.rotated ? part.height : part.width;
                const double height = part.rotated ? part.width : part.height;
                const auto found = std::find_if(items.begin(), items.end(), [&](const Rect& item)
                { return item.partId == part.partId && item.width == width && item.height == height; });
                if (found == items.end()) items.push_back({ width, height, 1, part.partId });
                else ++found->quantity;
            }
    }
    std::vector<Sheet> sheets;
    std::vector<Rect> items;
    std::vector<NestingGeometry::Part> geometryParts;
    bool fitAll = true;
    bool explicitItems = false;
    double zoom = 1;
    int scroll = 0, maxScroll = 0;
    SDL_Rect canvas{}, allButton{}, widthButton{}, minusButton{}, plusButton{};
};
