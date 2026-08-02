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

void Button::update(const SDL_Event& e) {
    // Track mouse hover position
    if (e.type == SDL_MOUSEMOTION) {
        int mx = e.motion.x;
        int my = e.motion.y;

        // Check if mouse is inside the button bounding box
        if (mx >= getX() && mx <= getX() + getWidth() &&
            my >= getY() && my <= getY() + getHeight()) {
            hovered = true;
        }
        else {
            hovered = false;
        }
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
    if (!visible)
        return;

    // Navigation buttons
    if (style == ButtonStyle::Navigation)
    {
        SDL_Color textColour = DefaultTheme.darkText;

        if (selected)
            textColour = DefaultTheme.accent;
        else if (hovered)
            textColour = DefaultTheme.navigationHover;

        renderer.drawText(
            text,
            bounds.x + 18,
            bounds.y + 10,
            textColour);

        return;
    }

    // Standard buttons
    SDL_Color backgroundColour = DefaultTheme.buttonNormal;

    if (selected)
    {
        backgroundColour = DefaultTheme.accent;
    }
    else if (pressed)
    {
        backgroundColour = DefaultTheme.buttonPressed;
    }
    else if (hovered)
    {
        backgroundColour = DefaultTheme.buttonHover;
    }

    renderer.fillRoundedRect(
        bounds,
        backgroundColour,
        5);

    SDL_Color textColour =
        selected
        ? DefaultTheme.lightText
        : DefaultTheme.darkText;

    renderer.drawText(
        text,
        bounds.x + 12,
        bounds.y + 10,
        textColour);
}

void Button::setSelected(bool value)
{
    selected = value;
}