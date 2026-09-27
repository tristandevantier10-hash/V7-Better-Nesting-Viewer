#include "CardPanel.h"
#include "Renderer.h"
#include "FontManager.h"
#include "VerticalLayout.h"

CardPanel::CardPanel()
{
    setStyle(PanelStyle::Card);
    setLayout(std::make_unique<VerticalLayout>());

    setPadding(24);
    setSpacing(12);

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
    // Flat white background
    renderer.fillRect(
        getBounds(),
        SDL_Color{ 255,255,255,255 });

    // Optional subtle border
    renderer.drawRect(
        getBounds(),
        SDL_Color{ 235,235,235,255 });

    // Title
    if (!title.empty())
    {
        renderer.drawText(
            title,
            getX() + 24,
            getY() + 20,
            renderer.getFontManager().getHeadingFont(),
            SDL_Color{ 45,45,45,255 });
    }

    performLayout();

    renderChildren(renderer);
}