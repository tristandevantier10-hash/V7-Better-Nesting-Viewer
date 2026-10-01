#include "Renderer.h"
#include "TextRenderer.h"
#include "Label.h"
#include <cmath>

Renderer::Renderer(
    SDL_Renderer* renderer,
    FontManager& fonts,
    Theme& theme)
    :
    sdlRenderer(renderer),
    fontManager(fonts),
    theme(theme),
    shapeRenderer(renderer)

{}

void Renderer::fillRect(
    const SDL_Rect& rect,
    SDL_Color colour)
{
    SDL_Rect translated = rect;

    translated.x += offsetX;
    translated.y += offsetY;

    shapeRenderer.fillRect(
        translated,
        colour);
}

void Renderer::fillRoundedRect(
    const SDL_Rect& rect,
    SDL_Color colour,
    int radius)
{
    SDL_Rect translated = rect;

    translated.x += offsetX;
    translated.y += offsetY;

    shapeRenderer.fillRoundedRect(
        translated,
        colour,
        radius);
}

void Renderer::fillCircle(
    int cx,
    int cy,
    int radius,
    SDL_Color colour)
{
    shapeRenderer.fillCircle(
        cx + offsetX,
        cy + offsetY,
        radius,
        colour);
}

void Renderer::drawRect(
    const SDL_Rect& rect,
    SDL_Color colour)
{
    SDL_Rect translated = rect;

    translated.x += offsetX;
    translated.y += offsetY;

    shapeRenderer.drawRect(
        translated,
        colour);
}

FontManager& Renderer::getFontManager()
{
    return fontManager;
}

void Renderer::drawText(
    const std::string& text,
    int x,
    int y)
{
    TextRenderer::drawText(
        sdlRenderer,
        text,
        x + offsetX,
        y + offsetY,
        theme.darkText
    );
}

void Renderer::drawText(
    const std::string& text,
    int x,
    int y,
    SDL_Color colour)
{
    TextRenderer::drawText(
        sdlRenderer,
        text,
        x + offsetX,
        y + offsetY,
        colour
    );
}

void Renderer::drawText(
    const std::string& text,
    int x,
    int y,
    TTF_Font* font,
    SDL_Color colour)
{
    if (font == nullptr)
        font = fontManager.getNormalFont();

    TextRenderer::drawText(
        sdlRenderer,
        font,
        text,
        x + offsetX,
        y + offsetY,
        colour);
}

void Renderer::drawText(
    const std::string& text,
    int x,
    int y,
    LabelStyle style,
    SDL_Color colour)
{
    TTF_Font* font = nullptr;

    switch (style)
    {
    case LabelStyle::Small:
        font = fontManager.getSmallFont();
        break;

    case LabelStyle::Heading:
        font = fontManager.getHeadingFont();
        break;

    default:
        font = fontManager.getNormalFont();
        break;
    }

    TextRenderer::drawText(
        sdlRenderer,
        font,
        text,
        x + offsetX,
        y + offsetY,
        colour);
}

void Renderer::drawLine(
    int x1,
    int y1,
    int x2,
    int y2,
    SDL_Color colour)
{
    shapeRenderer.drawLine(
        x1 + offsetX,
        y1 + offsetY,
        x2 + offsetX,
        y2 + offsetY,
        colour);
}

void Renderer::beginFrame()
{
    SDL_SetRenderDrawColor(
        sdlRenderer,
        theme.windowBackground.r,
        theme.windowBackground.g,
        theme.windowBackground.b,
        theme.windowBackground.a);

    SDL_RenderClear(sdlRenderer);
}

void Renderer::endFrame()
{
    SDL_RenderPresent(sdlRenderer);
}

SDL_Renderer* Renderer::getSDLRenderer()
{
    return sdlRenderer;
}

void Renderer::pushOffset(int x, int y)
{
    offsetStack.push_back({ offsetX, offsetY });

    offsetX += x;
    offsetY += y;
}

void Renderer::popOffset()
{
    if (offsetStack.empty())
        return;

    SDL_Point p = offsetStack.back();

    offsetStack.pop_back();

    offsetX = p.x;
    offsetY = p.y;
}

void Renderer::pushClip(const SDL_Rect& rect)
{
    clipStack.push_back(rect);

    SDL_RenderSetClipRect(
        sdlRenderer,
        &clipStack.back());
}

void Renderer::popClip()
{
    if (!clipStack.empty())
    {
        clipStack.pop_back();
    }

    if (clipStack.empty())
    {
        SDL_RenderSetClipRect(
            sdlRenderer,
            nullptr);
    }
    else
    {
        SDL_RenderSetClipRect(
            sdlRenderer,
            &clipStack.back());
    }
}

int Renderer::getTextWidth(const std::string& text)
{
    return TextRenderer::getTextWidth(text);
}

void Renderer::drawRoundedRect(
    const SDL_Rect& rect,
    SDL_Color colour,
    int radius)
{
    SDL_Rect translated = rect;

    translated.x += offsetX;
    translated.y += offsetY;

    shapeRenderer.drawRoundedRect(
        translated,
        colour,
        radius);
}

void Renderer::drawRoundedRing(
    const SDL_Rect& rect,
    SDL_Color colour,
    int radius,
    int thickness)
{
    SDL_Rect translated = rect;

    translated.x += offsetX;
    translated.y += offsetY;

    shapeRenderer.drawRoundedRing(
        translated,
        colour,
        radius,
        thickness);
}