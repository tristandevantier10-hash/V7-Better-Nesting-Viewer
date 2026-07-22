#pragma once

#include <SDL2/SDL_ttf.h>
#include <string>

class FontManager
{
public:

    FontManager();

    ~FontManager();

    bool initialise();

    void shutdown();

    TTF_Font* getSmallFont() const;
    TTF_Font* getNormalFont() const;
    TTF_Font* getHeadingFont() const;

private:

    TTF_Font* smallFont = nullptr;

    TTF_Font* normalFont = nullptr;

    TTF_Font* headingFont = nullptr;
};
