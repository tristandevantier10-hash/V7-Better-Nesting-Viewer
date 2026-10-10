#include "TextBox.h"
#include "Renderer.h"
#include "Theme.h"
#include "TextRenderer.h"
#include "Label.h"

TextBox::TextBox()
{
    setSize(200, 32);
}

void TextBox::setText(const std::string& value)
{
    text = value;
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

void TextBox::update(const SDL_Event& e)
{
    Uint32 now = SDL_GetTicks();

    if (now - lastBlink > 500)
    {
        showCaret = !showCaret;
        lastBlink = now;
    }

    if (e.type == SDL_MOUSEBUTTONDOWN &&
        e.button.button == SDL_BUTTON_LEFT)
    {
        int mx = e.button.x;
        int my = e.button.y;

        SDL_Rect r = getBounds();

        focused =
            mx >= r.x &&
            mx < r.x + r.w &&
            my >= r.y &&
            my < r.y + r.h;
    }

    if (focused &&
        e.type == SDL_TEXTINPUT)
    {
        text += e.text.text;

        if (textChangedCallback)
            textChangedCallback(text);
    }

    if (focused &&
        e.type == SDL_KEYDOWN)
    {
        if (e.key.keysym.sym == SDLK_BACKSPACE &&
            !text.empty())
        {
            // Remove the last UTF-8 character.
            size_t pos = text.find_last_of(
                "\xC0");

            (void)pos;

            size_t i = text.size() - 1;

            while (i > 0 &&
                (static_cast<unsigned char>(text[i]) & 0xC0) == 0x80)
            {
                --i;
            }

            text.erase(i);

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
        8);

    SDL_Color borderColour =
        focused
        ? DefaultTheme.accent
        : DefaultTheme.border;

    renderer.drawRoundedRing(
        getBounds(),
        borderColour,
        8,
        focused ? 2 : 1);

    std::string displayText = text;

    if (passwordMode)
        displayText = std::string(text.length(), '*');

    if (text.empty())
    {
        renderer.drawText(
            placeholder,
            getX() + 8,
            getY() + 8,
            LabelStyle::Normal,
            SDL_Color{ 150,150,150,255 });
    }
    else
    {
        renderer.drawText(
            displayText,
            getX() + 8,
            getY() + 8,
            LabelStyle::Normal,
            DefaultTheme.darkText);
    }

    if (focused && showCaret)
    {
        int caretX =
            bounds.x + 8 +
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