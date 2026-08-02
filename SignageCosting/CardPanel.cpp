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

    SDL_Rect shadow =
    {
        getX() + 3,
        getY() + 3,
        getWidth(),
        getHeight()
    };

    renderer.fillRoundedRect(
        shadow,
        SDL_Color{ 100,100,100,255 },
        14);

    // Draw the card
    renderer.fillRoundedRect(
        getBounds(),
        SDL_Color{ 248,248,248,255 },
        14);

    renderer.drawRoundedRect(
        getBounds(),
        SDL_Color{ 225,225,225,255 },
        14);

    // Draw title
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