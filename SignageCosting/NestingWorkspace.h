#pragma once

// Workspace controls; sheet previews share their presentation with quote results.
#include "NestingStyle.h"
#include "NestingPreviewPanel.h"
#include "Panel.h"
#include "Button.h"
#include "Renderer.h"
#include "FontManager.h"
#include "Label.h"
#include "NestingEngine.h"
#include <array>
#include <cmath>
#include <functional>
#include <iomanip>
#include <sstream>

class NestingWorkspacePanel : public Panel
{
public:
    std::function<void()> arrange;
    std::function<void(Renderer&)> paint;

    void render(Renderer& renderer) override
    {
        if (arrange) arrange();
        if (paint) paint(renderer);
        renderChildren(renderer);
    }
    void update(const SDL_Event& event) override
    {
        if (arrange) arrange();
        Panel::update(event);
    }
};

class NestingPageButton : public Button
{
public:
    explicit NestingPageButton(std::string label, bool primary = false)
        : caption(std::move(label)), primary(primary) {}
    void setCaption(const std::string& label) { caption=label; }
    void update(const SDL_Event& event) override
    {
        if (event.type == SDL_MOUSEMOTION)
        {
            SDL_Point mouse{ event.motion.x, event.motion.y };
            over = SDL_PointInRect(&mouse, &bounds);
        }
        Button::update(event);
    }
    void render(Renderer& renderer) override
    {
        using namespace NestingStyle;
        const auto fill = primary ? (over ? SDL_Color{ 29, 123, 65, 255 } : green)
            : (over ? SDL_Color{ 246, 249, 252, 255 } : white);
        renderer.fillRoundedRect(bounds, fill, 7);
        renderer.drawRoundedRect(bounds, primary ? green : border, 7);
        auto* font = primary ? renderer.getFontManager().getNormalFont()
            : renderer.getFontManager().getSmallFont();
        int width = 0, height = 0;
        TTF_SizeUTF8(font, caption.c_str(), &width, &height);
        if (primary && width + 20 > bounds.w)
        {
            font = renderer.getFontManager().getSmallFont();
            TTF_SizeUTF8(font, caption.c_str(), &width, &height);
        }
        const int x = bounds.x + (bounds.w - width) / 2;
        renderer.pushClip(bounds);
        renderer.drawText(caption, x, bounds.y + (bounds.h - height) / 2,
            font, primary ? white : ink);
        if (primary && x - bounds.x > 32)
            renderer.drawSVG("Assets/Icons/nesting.svg",
                { x - 28, bounds.y + (bounds.h - 18) / 2, 18, 18 }, white);
        renderer.popClip();
    }
private:
    std::string caption;
    bool primary = false, over = false;
};

class NestingSizeSwatch : public UIElement
{
public:
    size_t index = 0;
    void render(Renderer& renderer) override
    {
        const int side = std::min(27, bounds.h - 6);
        renderer.fillRoundedRect({ bounds.x + 7, bounds.y + (bounds.h - side) / 2,
            side, side }, NestingStyle::colours[index % NestingStyle::colours.size()], 5);
    }
};

