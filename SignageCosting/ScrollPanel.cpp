#include "ScrollPanel.h"
#include "Renderer.h"
#include "iostream"

ScrollPanel::ScrollPanel()
{}

void ScrollPanel::render(Renderer& renderer)
{
    SDL_Rect clip =
    {
        getX(),
        getY(),
        getWidth(),
        getHeight()
    };

    renderer.pushClip(clip);

    renderer.pushOffset(0, -scrollY);

    Panel::render(renderer);

    renderer.popOffset();

    renderer.popClip();
}

void ScrollPanel::update(const SDL_Event& e)
{
    if (e.type == SDL_MOUSEWHEEL)
    {
        scrollY -= e.wheel.y * scrollSpeed;

        if (scrollY < 0)
            scrollY = 0;

        std::cout
            << "Scroll Y = "
            << scrollY
            << std::endl;
    }

    Panel::update(e);
}