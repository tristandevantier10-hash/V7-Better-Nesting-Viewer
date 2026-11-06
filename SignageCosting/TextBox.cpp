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
    selectionAnchor = selectionEnd = text.size();
    draggingSelection = false;
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

void TextBox::setSelectAllOnFocus(bool enabled)
{
    selectAllOnFocus = enabled;
}

size_t TextBox::textIndexAt(int mouseX) const
{
    const auto display = passwordMode ? std::string(text.size(), '*') : text;
    const int padding = Metrics::ControlPaddingX;
    const int left = getX() + padding + (textCentered
        ? std::max(0, (getWidth() - 2 * padding - TextRenderer::getTextWidth(display)) / 2) : 0);
    int previousWidth = 0;
    for (size_t index = 0; index < text.size();)
    {
        size_t next = index + 1;
        while (next < text.size() && (static_cast<unsigned char>(text[next]) & 0xC0) == 0x80)
            ++next;
        const int width = TextRenderer::getTextWidth(display.substr(0, next));
        if (mouseX - left < (previousWidth + width) / 2) return index;
        previousWidth = width;
        index = next;
    }
    return text.size();
}

void TextBox::update(const SDL_Event& e)
{
    if (!isEnabled() || ComboBox::hasOpenDropdown())
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
        const bool gainedFocus = clickedInside && !hasFocus();
        setFocused(clickedInside);
        allTextSelected = clickedInside && !text.empty() && selectAllOnFocus &&
            (gainedFocus || e.button.clicks >= 2);
        draggingSelection = clickedInside;
        selectionAnchor = selectionEnd = clickedInside ? textIndexAt(mx) : text.size();
    }

    if (hasFocus() && draggingSelection && e.type == SDL_MOUSEMOTION &&
        (e.motion.state & SDL_BUTTON_LMASK))
    {
        selectionEnd = textIndexAt(e.motion.x);
        if (selectionEnd != selectionAnchor) allTextSelected = false;
    }
    if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT)
        draggingSelection = false;

    const auto eraseSelection = [this]()
    {
        if (!allTextSelected && selectionAnchor == selectionEnd) return false;
        const size_t start = allTextSelected ? 0 : std::min(selectionAnchor, selectionEnd);
        const size_t end = allTextSelected ? text.size() : std::max(selectionAnchor, selectionEnd);
        text.erase(start, end - start);
        allTextSelected = false;
        selectionAnchor = selectionEnd = start;
        return true;
    };

    if (hasFocus() &&
        e.type == SDL_TEXTINPUT)
    {
        eraseSelection();
        text.insert(selectionEnd, e.text.text);
        selectionEnd += std::char_traits<char>::length(e.text.text);
        selectionAnchor = selectionEnd;

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
            if (!eraseSelection())
            {
                size_t start = selectionEnd, end = selectionEnd;
                if (e.key.keysym.sym == SDLK_BACKSPACE && start > 0)
                {
                    do { --start; }
                    while (start > 0 && (static_cast<unsigned char>(text[start]) & 0xC0) == 0x80);
                }
                else if (e.key.keysym.sym == SDLK_DELETE && end < text.size())
                {
                    do { ++end; }
                    while (end < text.size() && (static_cast<unsigned char>(text[end]) & 0xC0) == 0x80);
                }
                if (start == end) return;
                text.erase(start, end - start);
                selectionAnchor = selectionEnd = start;
            }

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
        isEnabled() ? SDL_Color{255,255,255,255} : SDL_Color{246,248,250,255},
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
    const bool selected = hasFocus() && !text.empty() &&
        (allTextSelected || selectionAnchor != selectionEnd);
    int selectionX = textX, selectionWidth = 0;
    renderer.pushClip(getBounds());
    if (selected)
    {
        const size_t start = allTextSelected ? 0 : std::min(selectionAnchor, selectionEnd);
        const size_t end = allTextSelected ? text.size() : std::max(selectionAnchor, selectionEnd);
        selectionX += TextRenderer::getTextWidth(displayText.substr(0, start));
        selectionWidth = std::max(1, textX + TextRenderer::getTextWidth(displayText.substr(0, end)) - selectionX);
        renderer.fillRect(
            { selectionX, textY, selectionWidth, Metrics::ControlTextHeight },
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
            DefaultTheme.darkText);
        if (selected)
        {
            renderer.pushClip({selectionX,textY,selectionWidth,Metrics::ControlTextHeight});
            renderer.drawText(displayText,textX,textY,LabelStyle::Normal,SDL_Color{255,255,255,255});
            renderer.popClip();
        }
    }

    if (hasFocus() && showCaret && !selected)
    {
        const int caretX = (text.empty() ? contentLeft : textX) +
            TextRenderer::getTextWidth(displayText.substr(0, selectionEnd));

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
    renderer.popClip();
}

void TextBox::setTextChangedCallback(
    std::function<void(const std::string&)> callback)
{
    textChangedCallback = callback;
}
