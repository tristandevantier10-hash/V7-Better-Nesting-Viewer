#pragma once

#include <SDL2/SDL.h>

class ShapeRenderer
{
public:

    ShapeRenderer(SDL_Renderer* renderer);

    void fillRect(
        const SDL_Rect& rect,
        SDL_Color colour);

    void drawRect(
        const SDL_Rect& rect,
        SDL_Color colour);

    void drawLine(
        int x1,
        int y1,
        int x2,
        int y2,
        SDL_Color colour);

    void fillCircle(
        int cx,
        int cy,
        int radius,
        SDL_Color colour);

    void fillRoundedRect(
        const SDL_Rect& rect,
        SDL_Color colour,
        int radius);

    void drawRoundedRect(
        const SDL_Rect& rect,
        SDL_Color colour,
        int radius);

    void drawRoundedRing(
        const SDL_Rect& rect,
        SDL_Color colour,
        int radius,
        int thickness = 1);

    void drawShadow(
        const SDL_Rect& rect,
        int radius);

private:

    SDL_Renderer* sdlRenderer = nullptr;
};