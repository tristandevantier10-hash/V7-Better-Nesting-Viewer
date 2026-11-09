#include "CardPanel.h"
#include "Renderer.h"
#include "FontManager.h"
#include "VerticalLayout.h"
#include "Metrics.h"

CardPanel::CardPanel()
{
    setStyle(PanelStyle::Card);
    setLayout(std::make_unique<VerticalLayout>());

    setPadding(Metrics::CardPadding);
    setSpacing(Metrics::SpaceS);

    headerHeight = 30;
    layout->setTopInset(headerHeight);
}

void CardPanel::setTitle(const std::string& text)
{
    title = text;
}

const std::string& CardPanel::getTitle() const
{
    return title;
}

void CardPanel::render(Renderer& renderer)
{
    renderBackground(renderer);

    // Title
    if (!title.empty())
    {
        renderer.drawText(
            title,
            getX() + Metrics::CardPadding,
            getY() + Metrics::CardPadding,
            renderer.getFontManager().getSectionFont(),
            SDL_Color{ 45,45,45,255 });
    }

    performLayout();

    renderChildren(renderer);
}
