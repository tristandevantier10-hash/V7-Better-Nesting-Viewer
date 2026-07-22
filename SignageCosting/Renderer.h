#pragma once

#include <SDL2/SDL.h>
#include <string>
#include "Theme.h"
#include <vector>

class FontManager;

class Renderer
{
public:

    Renderer(
        SDL_Renderer* renderer,
        FontManager& fonts,
        Theme& theme);

    //============================
    // Frame
    //============================

    void beginFrame();

    void endFrame();

    //============================
    // Primitive drawing
    //============================

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

    //============================
    // Scroll
    //============================

    void pushOffset(int x, int y);

    void popOffset();

    //============================
    // Clipping
    //============================

    void pushClip(const SDL_Rect& rect);

    void popClip();

    //============================
    // Text
    //============================

    void drawText(
        const std::string& text,
        int x,
        int y);

    //============================
    // Temporary bridge
    //============================

    SDL_Renderer* getSDLRenderer();

    int getTextWidth(const std::string& text);

private:

    SDL_Renderer* sdlRenderer;

    FontManager& fontManager;

    Theme& theme;

    int offsetX = 0;

    int offsetY = 0;

    std::vector<SDL_Point> offsetStack;

    std::vector<SDL_Rect> clipStack;
};
