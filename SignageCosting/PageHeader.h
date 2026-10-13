#pragma once

#include "Panel.h"
#include "Metrics.h"
#include "Renderer.h"
#include "Label.h"
#include <algorithm>
#include <memory>

inline constexpr int PageHeaderLogoWidth = 190;

class PageHeaderPanel : public Panel
{
protected:
    void renderBackground(Renderer& renderer) override
    {
        Panel::renderBackground(renderer);
        const SDL_Rect bounds = getBounds();
        renderer.fillRect(
            { bounds.x, bounds.y + bounds.h - 3, bounds.w, 3 },
            SDL_Color{ 137, 190, 58, 255 });
    }
};

class PageHeaderContentPanel : public Panel
{
protected:
    void renderBackground(Renderer&) override
    {
        // Header content floats over the optional banner image.
    }
};

class PageHeaderLogo : public UIElement
{
public:
    void render(Renderer& renderer) override
    {
        if (!isVisible())
            return;

        SDL_Rect logoBounds = getBounds();
        logoBounds.x += 8;
        logoBounds.y += 8;
        logoBounds.w = std::max(0, logoBounds.w - 16);
        logoBounds.h = std::max(0, logoBounds.h - 16);
        renderer.drawImage("Assets/Images/EstiMate.png", logoBounds);
    }
};

inline void configurePageHeader(
    const std::shared_ptr<Panel>& panel,
    int padding = Metrics::SpaceM,
    int spacing = Metrics::SpaceM)
{
    panel->setHeaderStyle(true);
    panel->setBorderVisible(false);
    panel->setBackgroundColour(SDL_Color{ 248, 249, 250, 255 });
    panel->setPadding(padding);
    panel->setSpacing(spacing);
}

inline void configurePageHeaderTitleLabel(const std::shared_ptr<Label>& label)
{
    label->setTextTheme(TextTheme::Dark);
}

inline void configurePageHeaderSubtitleLabel(const std::shared_ptr<Label>& label)
{
    label->setTextTheme(TextTheme::DarkSecondary);
}

inline void configurePageHeaderTitle(
    const std::shared_ptr<Panel>& panel,
    int spacing = Metrics::SpaceXS)
{
    panel->setHeaderStyle(true);
    panel->setBorderVisible(false);
    panel->setPadding(0);
    panel->setSpacing(spacing);
}

inline void addPageHeaderLogo(const std::shared_ptr<Panel>& panel)
{
    panel->addLayoutElement(
        std::make_shared<PageHeaderLogo>(),
        SizePolicy::Fixed,
        PageHeaderLogoWidth);
}
