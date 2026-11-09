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

namespace
{
    bool insideRoundedRect(float x, float y, int width, int height, int radius)
    {
        if (x < 0 || y < 0 || x >= width || y >= height)
            return false;

        const float dx = std::max({radius - x, x - (width - radius), 0.0f});
        const float dy = std::max({radius - y, y - (height - radius), 0.0f});
        return dx * dx + dy * dy <= static_cast<float>(radius) * radius;
    }

    void paintRoundedRect(SDL_Renderer* renderer, const SDL_Rect& rect,
        SDL_Color colour, int radius, int inset)
    {
        if (rect.w <= 0 || rect.h <= 0 || colour.a == 0)
            return;

        radius = std::clamp(radius, 0, std::min(rect.w, rect.h) / 2);
        const bool ring = inset > 0;
        inset = std::min(inset, (std::min(rect.w, rect.h) + 1) / 2);
        const int edge = std::max(radius, inset);
        const int innerRadius = std::max(0, radius - inset);
        const int innerWidth = rect.w - 2 * inset;
        const int innerHeight = rect.h - 2 * inset;

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        const auto fill = [&](int x, int y, int width, int height)
        {
            if (width <= 0 || height <= 0) return;
            SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, colour.a);
            const SDL_Rect span{rect.x + x, rect.y + y, width, height};
            SDL_RenderFillRect(renderer, &span);
        };

        // Straight runs are solid and need no sampling.
        if (ring)
        {
            fill(edge, 0, rect.w - 2 * edge, inset);
            const int bottom = std::max(inset, rect.h - inset);
            fill(edge, bottom, rect.w - 2 * edge, rect.h - bottom);
            fill(0, edge, inset, rect.h - 2 * edge);
            const int right = std::max(inset, rect.w - inset);
            fill(right, edge, rect.w - right, rect.h - 2 * edge);
        }
        else
        {
            fill(edge, 0, rect.w - 2 * edge, rect.h);
            fill(0, edge, edge, rect.h - 2 * edge);
            fill(rect.w - edge, edge, edge, rect.h - 2 * edge);
        }

        // Sample only the corner blocks and mirror the coverage across the box.
        constexpr int samples = 4;
        constexpr int sampleCount = samples * samples;
        for (int y = 0; y < edge; ++y)
        {
            for (int x = 0; x < edge; ++x)
            {
                int covered = 0;
                for (int sy = 0; sy < samples; ++sy)
                    for (int sx = 0; sx < samples; ++sx)
                    {
                        const float px = x + (sx + 0.5f) / samples;
                        const float py = y + (sy + 0.5f) / samples;
                        if (insideRoundedRect(px, py, rect.w, rect.h, radius) &&
                            (!ring || !insideRoundedRect(px - inset, py - inset,
                                innerWidth, innerHeight, innerRadius)))
                            ++covered;
                    }

                if (!covered) continue;
                const auto alpha = static_cast<Uint8>((colour.a * covered + sampleCount / 2) / sampleCount);
                SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, alpha);
                const int left = rect.x + x, right = rect.x + rect.w - x - 1;
                const int top = rect.y + y, bottom = rect.y + rect.h - y - 1;
                SDL_RenderDrawPoint(renderer, left, top);
                if (right != left) SDL_RenderDrawPoint(renderer, right, top);
                if (bottom != top)
                {
                    SDL_RenderDrawPoint(renderer, left, bottom);
                    if (right != left) SDL_RenderDrawPoint(renderer, right, bottom);
                }
            }
        }
    }
}

void ShapeRenderer::fillRoundedRect(const SDL_Rect& rect, SDL_Color colour, int radius)
{
    paintRoundedRect(sdlRenderer, rect, colour, radius, 0);
}

void ShapeRenderer::drawRoundedRect(const SDL_Rect& rect, SDL_Color colour, int radius)
{
    drawRoundedRing(rect, colour, radius, 1);
}

void ShapeRenderer::drawRoundedRing(const SDL_Rect& rect, SDL_Color colour, int radius, int thickness)
{
    if (thickness > 0)
        paintRoundedRect(sdlRenderer, rect, colour, radius, thickness);
}
