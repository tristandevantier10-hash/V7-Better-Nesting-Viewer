#pragma once

#include "Panel.h"
#include "Metrics.h"
#include "Renderer.h"
#include "Label.h"
#include "HorizontalLayout.h"
#include "VerticalLayout.h"
#include "Theme.h"
#include <algorithm>
#include <memory>
#include <string>
#include <utility>

inline constexpr int PageHeaderLogoWidth = 190;

class PageHeaderPanel : public Panel
{
public:
    void setDividerVisible(bool visible)
    {
        dividerVisible = visible;
    }

    void setBottomAccentVisible(bool visible)
    {
        bottomAccentVisible = visible;
    }

    void setLayout(std::unique_ptr<Layout> newLayout)
    {
        if (auto horizontal = dynamic_cast<HorizontalLayout*>(newLayout.get()))
            horizontal->setCenterChildrenVertically(true);

        Panel::setLayout(std::move(newLayout));
    }

protected:
    void renderBackground(Renderer& renderer) override
    {
        Panel::renderBackground(renderer);
        const SDL_Rect bounds = getBounds();
        int dividerX = bounds.x + getPadding() + PageHeaderLogoWidth + getSpacing() / 2;
        if (children.size() > 1)
        {
            const SDL_Rect logoCell = children.front()->getBounds();
            const int renderedLogoRight = logoCell.x + logoCell.w - 12;
            int titleStartX = children[1]->getX();
            if (const auto titlePanel = std::dynamic_pointer_cast<Panel>(children[1]))
                titleStartX += titlePanel->getPadding();

            dividerX = (renderedLogoRight + titleStartX) / 2;
        }
        if (dividerVisible)
        {
            renderer.drawLine(
                dividerX,
                bounds.y + 22,
                dividerX,
                bounds.y + bounds.h - 22,
                DefaultTheme.border);
        }
        if (bottomAccentVisible)
        {
            renderer.fillRect(
                { bounds.x, bounds.y + bounds.h - 2, bounds.w, 2 },
                DefaultTheme.accent);
        }
    }

private:
    bool dividerVisible = true;
    bool bottomAccentVisible = true;
};

class PageHeaderContentPanel : public Panel
{
public:
    void addLayoutElement(
        std::shared_ptr<UIElement> element,
        SizePolicy policy = SizePolicy::Fill,
        int size = 0)
    {
        if (policy == SizePolicy::Fixed)
        {
            if (const auto label = std::dynamic_pointer_cast<Label>(element))
            {
                switch (label->getStyle())
                {
                case LabelStyle::Heading: size = 30; break;
                case LabelStyle::Small: size = 18; break;
                case LabelStyle::Normal: size = 22; break;
                }
            }
        }

        Panel::addLayoutElement(std::move(element), policy, size);
    }

    void setLayout(std::unique_ptr<Layout> newLayout)
    {
        if (auto vertical = dynamic_cast<VerticalLayout*>(newLayout.get()))
            vertical->setCenterContent(true);

        Panel::setLayout(std::move(newLayout));
    }

protected:
    void renderBackground(Renderer&) override
    {
        // Header content floats over the optional banner image.
    }
};

class PageHeaderLogo : public UIElement
{
public:
    explicit PageHeaderLogo(
        std::string imagePath = "Assets/Images/EstiMate.png",
        float scale = 1.0f)
        : imagePath(std::move(imagePath)), scale(scale)
    {
    }

    void render(Renderer& renderer) override
    {
        if (!isVisible())
            return;

        SDL_Rect logoBounds = getBounds();
        logoBounds.x += 12;
        logoBounds.y += 12;
        logoBounds.w = std::max(0, logoBounds.w - 24);
        logoBounds.h = std::max(0, logoBounds.h - 24);
        const int scaledWidth = static_cast<int>(logoBounds.w * scale);
        const int scaledHeight = static_cast<int>(logoBounds.h * scale);
        logoBounds.x += (logoBounds.w - scaledWidth) / 2;
        logoBounds.y += (logoBounds.h - scaledHeight) / 2;
        logoBounds.w = scaledWidth;
        logoBounds.h = scaledHeight;
        renderer.drawImage(imagePath, logoBounds);
    }

private:
    std::string imagePath;
    float scale = 1.0f;
};

class DashboardHeaderControls : public UIElement
{
public:
    void render(Renderer& renderer) override
    {
        if (!isVisible())
            return;

        const int centerY = bounds.y + bounds.h / 2;
        const SDL_Rect statusPill{ bounds.x, centerY - 17, 148, 34 };
        renderer.fillRoundedRect(statusPill, SDL_Color{ 247, 250, 247, 255 }, 17);
        renderer.drawRoundedRect(statusPill, SDL_Color{ 224, 233, 221, 255 }, 17);
        renderer.fillCircle(bounds.x + 16, centerY, 4, SDL_Color{ 91, 157, 72, 255 });
        renderer.drawText("Workspace active", bounds.x + 28, centerY - 7,
            LabelStyle::Small, SDL_Color{ 75, 91, 76, 255 });

        const int dividerX = bounds.x + 164;
        renderer.drawLine(dividerX, centerY - 17, dividerX, centerY + 17,
            SDL_Color{ 229, 232, 235, 255 });

        const int avatarX = dividerX + 15;
        renderer.fillCircle(avatarX + 17, centerY, 17, SDL_Color{ 235, 242, 231, 255 });
        renderer.drawText("EG", avatarX + 7, centerY - 7,
            LabelStyle::Small, SDL_Color{ 62, 94, 54, 255 });
        renderer.drawLine(avatarX + 42, centerY - 2, avatarX + 46, centerY + 2,
            SDL_Color{ 111, 122, 130, 255 });
        renderer.drawLine(avatarX + 46, centerY + 2, avatarX + 50, centerY - 2,
            SDL_Color{ 111, 122, 130, 255 });
    }
};

class DashboardBannerPanel : public PageHeaderPanel
{
protected:
    void renderBackground(Renderer& renderer) override
    {
        Panel::renderBackground(renderer);

        const SDL_Rect bounds = getBounds();
        renderer.drawImageCoverRegion(
            "Assets/Images/Header Background.png",
            1.0f / 3.0f, 0.0f, 2.0f / 3.0f, 1.0f,
            bounds);

        renderer.fillRect(
            { bounds.x, bounds.y + bounds.h - 2, bounds.w, 2 },
            DefaultTheme.accent);
    }
};

inline void configurePageHeader(
    const std::shared_ptr<Panel>& panel,
    int padding = Metrics::SpaceM,
    int spacing = Metrics::SpaceXL)
{
    panel->setHeaderStyle(true);
    panel->setBorderVisible(false);
    panel->setBackgroundColour(SDL_Color{ 255, 255, 255, 255 });
    panel->setPadding(padding);
    panel->setSpacing(spacing);
}

inline void configurePageHeaderTitleLabel(const std::shared_ptr<Label>& label)
{
    label->setTextTheme(TextTheme::Dark);
    label->setAlignment(LabelAlignment::Left);
}

inline void configurePageHeaderSubtitleLabel(const std::shared_ptr<Label>& label)
{
    label->setTextTheme(TextTheme::DarkSecondary);
    label->setAlignment(LabelAlignment::Left);
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

inline void addPageHeaderLogo(
    const std::shared_ptr<Panel>& panel,
    const std::string& imagePath = "Assets/Images/EstiMate.png",
    float scale = 1.0f)
{
    panel->addLayoutElement(
        std::make_shared<PageHeaderLogo>(imagePath, scale),
        SizePolicy::Fixed,
        PageHeaderLogoWidth);
}
