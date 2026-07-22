#include "Button.h"
#include "Renderer.h"
#include "iostream"

Button::Button()
{}

void Button::setText(const std::string& newText)
{
    text = newText;
}

void Button::setOnClick(std::function<void()> callback)
{
    onClick = callback;
}

void Button::update(const SDL_Event& e)
{
    if (!visible || !enabled)
        return;

    if (e.type == SDL_MOUSEMOTION)
    {
        SDL_Point mouse{
            e.motion.x,
            e.motion.y
        };

        hovered = SDL_PointInRect(&mouse, &bounds);
    }

    if (e.type == SDL_MOUSEBUTTONDOWN)
    {
        if (hovered)
        {
            pressed = true;
        }
    }

    if (e.type == SDL_MOUSEBUTTONUP)
    {
        if (pressed && hovered)
        {
            if (onClick)
                onClick();
        }

        pressed = false;
    }
}

void Button::render(Renderer& renderer)
{

    /*std::cout
        << "Button '" << text
        << "' at "
        << getX()
        << ", "
        << getY()
        << std::endl;
        */

    if (!visible)
        return;

    SDL_Color colour = DefaultTheme.buttonNormal;

    if (pressed)
        colour = DefaultTheme.buttonPressed;
    else if (hovered)
        colour = DefaultTheme.buttonHover;

    SDL_SetRenderDrawColor(
        renderer.getSDLRenderer(),
        colour.r,
        colour.g,
        colour.b,
        colour.a
    );

    SDL_RenderFillRect(
        renderer.getSDLRenderer(),
        &bounds
    );

    SDL_SetRenderDrawColor(
        renderer.getSDLRenderer(),
        DefaultTheme.border.r,
        DefaultTheme.border.g,
        DefaultTheme.border.b,
        DefaultTheme.border.a
    );

    SDL_RenderDrawRect(
        renderer.getSDLRenderer(),
        &bounds
    );

    renderer.drawText(
        text,
        bounds.x + 12,
        bounds.y + 10
    );
}



