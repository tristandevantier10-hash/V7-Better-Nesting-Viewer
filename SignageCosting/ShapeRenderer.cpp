#include "ShapeRenderer.h"
#include <cmath>
#include <algorithm>

ShapeRenderer::ShapeRenderer(SDL_Renderer* renderer)
    : sdlRenderer(renderer)
{}

void ShapeRenderer::fillRect(
    const SDL_Rect& rect,
    SDL_Color colour)
{
    SDL_SetRenderDrawColor(
        sdlRenderer,
        colour.r,
        colour.g,
        colour.b,
        colour.a);

    SDL_RenderFillRect(
        sdlRenderer,
        &rect);
}

void ShapeRenderer::drawRect(
    const SDL_Rect& rect,
    SDL_Color colour)
{
    SDL_SetRenderDrawColor(
        sdlRenderer,
        colour.r,
        colour.g,
        colour.b,
        colour.a);

    SDL_RenderDrawRect(
        sdlRenderer,
        &rect);
}

void ShapeRenderer::drawLine(
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
        x1,
        y1,
        x2,
        y2);
}

void ShapeRenderer::fillCircle(
    int cx,
    int cy,
    int radius,
    SDL_Color colour)
{
    SDL_SetRenderDrawBlendMode(
        sdlRenderer,
        SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(
        sdlRenderer,
        colour.r,
        colour.g,
        colour.b,
        colour.a);

    for (int y = -radius; y <= radius; ++y)
    {
        int dx = static_cast<int>(
            std::sqrt(radius * radius - y * y));

        SDL_RenderDrawLine(
            sdlRenderer,
            cx - dx,
            cy + y,
            cx + dx,
            cy + y);
    }
}

void ShapeRenderer::fillRoundedRect(
    const SDL_Rect& rect,
    SDL_Color colour,
    int radius)
{
    SDL_SetRenderDrawBlendMode(
        sdlRenderer,
        SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(
        sdlRenderer,
        colour.r,
        colour.g,
        colour.b,
        colour.a);

    // Centre rectangle
    SDL_Rect middle =
    {
        rect.x + radius,
        rect.y,
        rect.w - radius * 2,
        rect.h
    };

    SDL_RenderFillRect(
        sdlRenderer,
        &middle);

    // Left rectangle
    SDL_Rect left =
    {
        rect.x,
        rect.y + radius,
        radius,
        rect.h - radius * 2
    };

    SDL_RenderFillRect(
        sdlRenderer,
        &left);

    // Right rectangle
    SDL_Rect right =
    {
        rect.x + rect.w - radius,
        rect.y + radius,
        radius,
        rect.h - radius * 2
    };

    SDL_RenderFillRect(
        sdlRenderer,
        &right);

    fillCircle(
        rect.x + radius,
        rect.y + radius,
        radius,
        colour);

    fillCircle(
        rect.x + rect.w - radius - 1,
        rect.y + radius,
        radius,
        colour);

    fillCircle(
        rect.x + radius,
        rect.y + rect.h - radius - 1,
        radius,
        colour);

    fillCircle(
        rect.x + rect.w - radius - 1,
        rect.y + rect.h - radius - 1,
        radius,
        colour);
}

void ShapeRenderer::drawRoundedRect(
    const SDL_Rect& rect,
    SDL_Color colour,
    int radius)
{
    SDL_SetRenderDrawBlendMode(
        sdlRenderer,
        SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(
        sdlRenderer,
        colour.r,
        colour.g,
        colour.b,
        colour.a);

    SDL_Rect r = rect;

    // Top
    SDL_RenderDrawLine(
        sdlRenderer,
        r.x + radius,
        r.y,
        r.x + r.w - radius,
        r.y);

    // Bottom
    SDL_RenderDrawLine(
        sdlRenderer,
        r.x + radius,
        r.y + r.h - 1,
        r.x + r.w - radius,
        r.y + r.h - 1);

    // Left
    SDL_RenderDrawLine(
        sdlRenderer,
        r.x,
        r.y + radius,
        r.x,
        r.y + r.h - radius);

    // Right
    SDL_RenderDrawLine(
        sdlRenderer,
        r.x + r.w - 1,
        r.y + radius,
        r.x + r.w - 1,
        r.y + r.h - radius);

    // Rounded corners
    for (int dy = 0; dy <= radius; ++dy)
    {
        int dx = static_cast<int>(
            std::sqrt(radius * radius - dy * dy));

        // Top-left
        SDL_RenderDrawPoint(
            sdlRenderer,
            r.x + radius - dx,
            r.y + radius - dy);

        // Top-right
        SDL_RenderDrawPoint(
            sdlRenderer,
            r.x + r.w - radius + dx - 1,
            r.y + radius - dy);

        // Bottom-left
        SDL_RenderDrawPoint(
            sdlRenderer,
            r.x + radius - dx,
            r.y + r.h - radius + dy - 1);

        // Bottom-right
        SDL_RenderDrawPoint(
            sdlRenderer,
            r.x + r.w - radius + dx - 1,
            r.y + r.h - radius + dy - 1);
    }
}

void ShapeRenderer::drawRoundedRing(
    const SDL_Rect& rect,
    SDL_Color colour,
    int radius,
    int thickness)
{
    for (int i = 0; i < thickness; ++i)
    {
        SDL_Rect r =
        {
            rect.x + i,
            rect.y + 2 + i,
            rect.w - i * 2,
            rect.h - i * 2
        };

        if (r.w <= 0 || r.h <= 0)
            break;

        drawRoundedRect(
            r,
            colour,
            std::max(1, radius - i));
    }
}
