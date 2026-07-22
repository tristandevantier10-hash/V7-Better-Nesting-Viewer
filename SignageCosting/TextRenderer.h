#pragma once

#include <string>
#include <SDL2/SDL.h>
#include "FontManager.h"
#include "Theme.h"

class TextRenderer
{
public:

    static bool initialise(FontManager* fonts);

    static void shutdown();

    static void drawText(
        SDL_Renderer* renderer,
        const std::string& text,
        int x,
        int y,
        SDL_Color colour = DefaultTheme.text);

    static int getTextWidth(const std::string& text);

private:

    static FontManager* fontManager;
};
