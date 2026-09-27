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

    if (style == ButtonStyle::FilterChip)
    {
        SDL_Color background =
            selected
            ? SDL_Color{ 235,250,235,255 }
        : SDL_Color{ 255,255,255,255 };

        SDL_Color border =
            selected
            ? DefaultTheme.accent
            : SDL_Color{ 220,220,220,255 };

        SDL_Color text =
            selected
            ? DefaultTheme.accent
            : DefaultTheme.darkSecondaryText;

        renderer.fillRoundedRect(
            bounds,
            background,
            7);

        renderer.drawRoundedRect(
            bounds,
            border,
            7);

        renderer.drawText(
            this->text,
            bounds.x + 18,
            bounds.y + 11,
            text);

        return;
    }

    // Primary action button
    if (style == ButtonStyle::Primary)
    {
        SDL_Color backgroundColour = { 20,20,20,255 };

        if (pressed)
        {
            backgroundColour = { 0,140,75,255 };
        }
        else if (hovered)
        {
            backgroundColour = DefaultTheme.accent;
        }

        renderer.fillRoundedRect(
            bounds,
            backgroundColour,
            7);

        renderer.drawText(
            text,
            bounds.x + 12,
            bounds.y + 10,
            DefaultTheme.lightText);

        return;
    }

    //==================================================
    // Standard Buttons
    //==================================================

    // Normal = black
    SDL_Color backgroundColour =
    { 20, 20, 20, 255 };

    // Hover = green
    if (hovered)
    {
        backgroundColour =
            DefaultTheme.accent;
    }

    // Pressed = darker green
    if (pressed)
    {
        backgroundColour =
        { 0, 150, 75, 255 };
    }

    // Selected = green
    if (selected)
    {
        backgroundColour =
            DefaultTheme.accent;
    }

    // Background
    renderer.fillRoundedRect(
        bounds,
        backgroundColour,
        7);

    // White text
    SDL_Color textColour =
    { 255, 255, 255, 255 };

    // Draw text
    int textWidth =
        renderer.getTextWidth(text);

    int textX =
        bounds.x +
        (bounds.w - textWidth) / 2;

    int textY =
        bounds.y +
        (bounds.h - 20) / 2;

    renderer.drawText(
        text,
        textX,
        textY,
        textColour);
}

void Button::setSelected(bool value)
{
    selected = value;
}