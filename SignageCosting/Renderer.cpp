#include "Renderer.h"
#include "TextRenderer.h"

Renderer::Renderer(
    SDL_Renderer* renderer,
    FontManager& fonts,
    Theme& theme)
    :
    sdlRenderer(renderer),
    fontManager(fonts),
    theme(theme)
{}

void Renderer::fillRect(
    const SDL_Rect& rect,
    SDL_Color colour)
{
    SDL_SetRenderDrawColor(
        sdlRenderer,
        colour.r,
        colour.g,
        colour.b,
        colour.a);

    SDL_Rect translated = rect;

    translated.x += offsetX;
    translated.y += offsetY;

    SDL_RenderFillRect(
        sdlRenderer,
        &translated);
}

void Renderer::drawRect(
    const SDL_Rect& rect,
    SDL_Color colour)
{
    SDL_SetRenderDrawColor(
        sdlRenderer,
        colour.r,
        colour.g,
        colour.b,
        colour.a
    );

    SDL_Rect translated = rect;

    translated.x += offsetX;
    translated.y += offsetY;

    SDL_RenderDrawRect(
        sdlRenderer,
        &translated);
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
        theme.text
    );
}

void Renderer::drawLine(
    int x1,
    int y1,
    int x2,
    int y2,
    SDL_Color colour)
{
    SDL_SetRenderDrawColor(
        sdlRenderer,
        colour.r,
        colour.g,
        colour.b,
        colour.a);

    SDL_RenderDrawLine(
        sdlRenderer,
        x1 + offsetX,
        y1 + offsetY,
        x2 + offsetX,
        y2 + offsetY);
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