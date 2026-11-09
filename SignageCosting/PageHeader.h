#pragma once

#include "Panel.h"
#include "NavigationPanel.h"
#include "Button.h"
#include "Metrics.h"
#include "FontManager.h"
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
    bool framed = false;
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
                case LabelStyle::PageTitle: size = 42; break;
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
    void renderBackground(Renderer& renderer) override
    {
        if (children.size() < 2)
            return;

        const auto titleLabel = std::dynamic_pointer_cast<Label>(children.front());
        const auto subtitleLabel = std::dynamic_pointer_cast<Label>(children[1]);
        if (!titleLabel || !subtitleLabel)
            return;

        const auto& fonts = renderer.getFontManager();
        auto* subtitleFont = fonts.getHeaderSubtitleFont();
        subtitleLabel->setCustomFont(subtitleFont);

        int titleWidth = 0;
        int subtitleWidth = 0;
        TTF_SizeUTF8(fonts.getPageTitleFont(), titleLabel->getText().c_str(), &titleWidth, nullptr);
        TTF_SizeUTF8(subtitleFont, subtitleLabel->getText().c_str(), &subtitleWidth, nullptr);
        const SDL_Rect title = titleLabel->getBounds();
        const SDL_Rect subtitle = children[1]->getBounds();
        // The title font has more space below its visible lettering.
        constexpr int opticalOffset = 5;
        const int ruleY = (title.y + title.h + subtitle.y) / 2 - opticalOffset;
        if (framed && title.w > 0)
        {
            renderer.drawRect(
                { title.x - 12, title.y - 10,
                    std::min(title.w, std::max(titleWidth, subtitleWidth) + 34),
                    subtitle.y + subtitle.h - title.y + 20 },
                SDL_Color{ 130, 135, 132, 255 });
        }
        if (title.w > 0)
        {
            renderer.fillRect(
                { title.x, ruleY, std::min(title.w, std::max(titleWidth, subtitleWidth) + 10), 1 },
                DefaultTheme.darkText);
        }
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

class DashboardBannerPanel : public PageHeaderPanel
{
protected:
    void renderBackground(Renderer& renderer) override
    {
        Panel::renderBackground(renderer);

        const SDL_Rect bounds = getBounds();
        renderer.drawImageCoverRegion(
            "Assets/Images/Header Background.png",
            1.0f / 3.0f, 0.258f, 2.0f / 3.0f, 0.257f,
            bounds);

        renderer.fillRect(
            { bounds.x, bounds.y + bounds.h - 2, bounds.w, 2 },
            DefaultTheme.accent);
    }
};

inline void configurePageHeader(
    const std::shared_ptr<Panel>& panel,
    int padding = 32,
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
    label->setStyle(LabelStyle::PageTitle);
    label->setTextTheme(TextTheme::Dark);
    label->setAlignment(LabelAlignment::Left);
}

inline void configurePageHeaderSubtitleLabel(const std::shared_ptr<Label>& label)
{
    label->setStyle(LabelStyle::Normal);
    label->setTextTheme(TextTheme::Dark);
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

inline void addPageHeaderTitle(
    const std::shared_ptr<Panel>& header,
    const std::string& heading,
    const std::string& description, bool framed = false)
{
    auto titlePanel = std::make_shared<PageHeaderContentPanel>();
    titlePanel->setLayout(std::make_unique<VerticalLayout>());
    configurePageHeaderTitle(titlePanel);
    titlePanel->framed = framed;

    auto title = std::make_shared<Label>();
    title->setText(heading);
    configurePageHeaderTitleLabel(title);
    titlePanel->addLayoutElement(title, SizePolicy::Fixed);

    auto subtitle = std::make_shared<Label>();
    subtitle->setText(description);
    configurePageHeaderSubtitleLabel(subtitle);
    titlePanel->addLayoutElement(subtitle, SizePolicy::Fixed);

    header->addLayoutElement(titlePanel, SizePolicy::Fill);
}

inline void addPageHeaderActions(
    const std::shared_ptr<Panel>& header,
    const std::shared_ptr<Button>& primary,
    const std::shared_ptr<Button>& secondary = nullptr)
{
    auto actions = std::make_shared<PageHeaderContentPanel>();
    actions->setLayout(std::make_unique<VerticalLayout>());
    actions->setSpacing(14);

    if (primary)
    {
        primary->setStyle(Button::ButtonStyle::HeaderPrimary);
        actions->addLayoutElement(primary, SizePolicy::Fixed, Metrics::ControlHeight);
    }
    if (secondary)
    {
        secondary->setStyle(Button::ButtonStyle::HeaderOutline);
        actions->addLayoutElement(secondary, SizePolicy::Fixed, Metrics::ControlHeight);
    }
    header->addLayoutElement(actions, SizePolicy::Fixed, 144);
}

class WorkspaceMasthead : public Panel
{
public:
    void render(Renderer& renderer) override
    {
        Panel::render(renderer);
        const SDL_Rect row = getBounds();
        renderer.fillRect(
            { row.x, row.y + row.h - 2, row.w, 2 },
            DefaultTheme.accent);
        renderer.fillRect(
            { row.x + Metrics::SidebarWidth - 1, row.y, 2, row.h },
            DefaultTheme.accent);
    }
};

inline std::shared_ptr<Panel> addWorkspaceLayout(
    Panel& screen,
    const std::shared_ptr<Panel>& header)
{
    screen.setLayout(std::make_unique<VerticalLayout>());

    auto masthead = std::make_shared<WorkspaceMasthead>();
    masthead->setLayout(std::make_unique<HorizontalLayout>());
    masthead->addLayoutElement(createWorkspaceBrand(), SizePolicy::Fixed, Metrics::SidebarWidth);
    masthead->addLayoutElement(header, SizePolicy::Fill);
    screen.addLayoutElement(masthead, SizePolicy::Fixed, Metrics::HeaderHeight);

    auto body = std::make_shared<Panel>();
    body->setLayout(std::make_unique<HorizontalLayout>());
    screen.addLayoutElement(body, SizePolicy::Fill);
    return body;
}
