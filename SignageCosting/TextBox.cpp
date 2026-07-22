#include "TextBox.h"
#include "Renderer.h"
#include "Theme.h"
#include "TextRenderer.h"

TextBox::TextBox()
{
    setSize(200, 32);
}

void TextBox::setText(const std::string& value)
{
    text = value;
}

const std::string& TextBox::getText() const
{
    return text;
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
    }

    if (focused &&
        e.type == SDL_KEYDOWN)
    {
        if (e.key.keysym.sym == SDLK_BACKSPACE &&
            !text.empty())
        {
            text.pop_back();
        }
    }
}

void TextBox::render(Renderer& renderer)
{
    if (!visible)
        return;

    SDL_Color colour;

    if (focused)
    {
        colour = { 70, 110, 220, 255 };
    }
    else
    {
        colour = DefaultTheme.panelBackground;
    }

    renderer.fillRect(
        getBounds(),
        colour);

    renderer.drawRect(
        getBounds(),
        DefaultTheme.border);

    renderer.drawText(
        text,
        getX() + 8,
        getY() + 8);

    if (focused && showCaret)
    {
        int caretX =
            bounds.x + 8 +
            TextRenderer::getTextWidth(text);

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