#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include "Theme.h"
#include "ShapeRenderer.h"
#include <vector>

class FontManager;

enum class LabelStyle;

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

    void fillRoundedRect(
        const SDL_Rect& rect,
        SDL_Color colour,
        int radius);

    void drawRoundedRect(
        const SDL_Rect& rect,
        SDL_Color colour,
        int radius);

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

    void drawRoundedRing(
        const SDL_Rect& rect,
        SDL_Color colour,
        int radius,
        int thickness = 1);

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

    FontManager& getFontManager();

    void drawText(
        const std::string& text,
        int x,
        int y);

    void drawText(
        const std::string& text,
        int x,
        int y,
        SDL_Color colour);

    void drawText(
        const std::string& text,
        int x,
        int y,
        TTF_Font* font,
        SDL_Color colour);

    void drawText(
        const std::string& text,
        int x,
        int y,
        LabelStyle style,
        SDL_Color colour);

    void drawSVG(
        const std::string& path,
        const SDL_Rect& destination,
        SDL_Color colour);

    //============================
    // Temporary bridge
    //============================

    SDL_Renderer* getSDLRenderer();

    int getTextWidth(const std::string& text);

private:

    SDL_Renderer* sdlRenderer;

    ShapeRenderer shapeRenderer;

    FontManager& fontManager;

    Theme& theme;

    int offsetX = 0;

    int offsetY = 0;

    std::vector<SDL_Point> offsetStack;

    std::vector<SDL_Rect> clipStack;
};
