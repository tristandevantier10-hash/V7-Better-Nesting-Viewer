#include "TextBox.h"
#include "ComboBox.h"
#include "Renderer.h"
#include "Theme.h"
#include "TextRenderer.h"
#include "Label.h"
#include "Metrics.h"
#include <algorithm>

TextBox::TextBox()
{
    setSize(200, Metrics::ControlHeight);
}

void TextBox::setText(const std::string& value)
{
    text = value;
    allTextSelected = false;
}

void TextBox::setPlaceholder(const std::string& value)
{
    placeholder = value;
}

const std::string& TextBox::getText() const
{
    return text;
}

void TextBox::setPasswordMode(bool enabled)
{
    passwordMode = enabled;
}

void TextBox::setTextCentered(bool centered)
{
    textCentered = centered;
}

void TextBox::update(const SDL_Event& e)
{
    if (ComboBox::hasOpenDropdown())
        return;

    Uint32 now = SDL_GetTicks();

    if (now - lastBlink > 500)
    {
        showCaret = !showCaret;
        lastBlink = now;
    }

    if (e.type == SDL_MOUSEBUTTONDOWN &&
        e.button.button == SDL_BUTTON_LEFT)
    {
        if (ComboBox::wasClickConsumed(e.button.timestamp))
            return;

        int mx = e.button.x;
        int my = e.button.y;

        SDL_Rect r = getBounds();

        const bool clickedInside =
            mx >= r.x &&
            mx < r.x + r.w &&
            my >= r.y &&
            my < r.y + r.h;
        setFocused(clickedInside);
        if (clickedInside)
            allTextSelected = false;
    }

    if (hasFocus() &&
        e.type == SDL_TEXTINPUT)
    {
        if (allTextSelected)
            text.clear();
        allTextSelected = false;
        text += e.text.text;

        if (textChangedCallback)
            textChangedCallback(text);
    }

    if (hasFocus() &&
        e.type == SDL_KEYDOWN)
    {
        const bool controlPressed = (e.key.keysym.mod & KMOD_CTRL) != 0;
        if (controlPressed && e.key.keysym.sym == SDLK_a)
        {
            allTextSelected = !text.empty();
        }
        else if ((e.key.keysym.sym == SDLK_BACKSPACE ||
                  e.key.keysym.sym == SDLK_DELETE) && !text.empty())
        {
            if (allTextSelected)
            {
                text.clear();
            }
            else if (e.key.keysym.sym == SDLK_BACKSPACE)
            {
                // Remove the last UTF-8 code point.
                size_t i = text.size() - 1;
                while (i > 0 &&
                    (static_cast<unsigned char>(text[i]) & 0xC0) == 0x80)
                {
                    --i;
                }
                text.erase(i);
            }
            allTextSelected = false;

            if (textChangedCallback)
                textChangedCallback(text);
        }
    }
}

void TextBox::render(Renderer& renderer)
{
    if (!visible)
        return;

    renderer.fillRoundedRect(
        getBounds(),
        { 255,255,255,255 },
        Metrics::ControlRadius);

    SDL_Color borderColour =
        hasFocus()
        ? DefaultTheme.accent
        : DefaultTheme.border;

    renderer.drawRoundedRing(
        getBounds(),
        borderColour,
        Metrics::ControlRadius,
        hasFocus() ? 2 : 1);

    std::string displayText = text;

    if (passwordMode)
        displayText = std::string(text.length(), '*');

    const std::string& alignedText = text.empty() ? placeholder : displayText;
    const int contentLeft = getX() + Metrics::ControlPaddingX;
    const int contentWidth = getWidth() - Metrics::ControlPaddingX * 2;
    int textX = contentLeft;

    if (textCentered)
    {
        textX += std::max(
            0,
            (contentWidth - TextRenderer::getTextWidth(alignedText)) / 2);
    }

    const int textY = getY() + (getHeight() - Metrics::ControlTextHeight) / 2;
    if (hasFocus() && allTextSelected && !text.empty())
    {
        renderer.fillRect(
            { textX, textY, std::max(1, TextRenderer::getTextWidth(displayText)), Metrics::ControlTextHeight },
            { 55, 115, 205, 255 });
    }

    if (text.empty())
    {
        renderer.drawText(
            placeholder,
            textX,
            textY,
            LabelStyle::Normal,
            SDL_Color{ 150,150,150,255 });
    }
    else
    {
        renderer.drawText(
            displayText,
            textX,
            textY,
            LabelStyle::Normal,
            hasFocus() && allTextSelected
                ? SDL_Color{ 255,255,255,255 }
                : DefaultTheme.darkText);
    }

    if (hasFocus() && showCaret && !allTextSelected)
    {
        int caretX = contentLeft +
            (textCentered
                ? std::max(
                    0,
                    (contentWidth - TextRenderer::getTextWidth(displayText)) / 2)
                : 0) +
            TextRenderer::getTextWidth(displayText);

        SDL_SetRenderDrawColor(
            renderer.getSDLRenderer(),
            0,
            0,
            0,
            255);

        SDL_RenderDrawLine(
            renderer.getSDLRenderer(),
            caretX,
            bounds.y + 6,
            caretX,
            bounds.y + bounds.h - 6);
    }
}

void TextBox::setTextChangedCallback(
    std::function<void(const std::string&)> callback)
{
    textChangedCallback = callback;
}
