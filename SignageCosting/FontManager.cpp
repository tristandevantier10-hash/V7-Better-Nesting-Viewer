#include "FontManager.h"
#include <SDL2/SDL_ttf.h>
#include <iostream>

FontManager::FontManager()
{}

FontManager::~FontManager()
{
    shutdown();
}

bool FontManager::initialise()
{
    if (TTF_Init() != 0)
        return false;

    smallFont =
        TTF_OpenFont(
            "Fonts/BasisGrotesqueArabicPro-Regular.ttf",
            14);

    if (!smallFont)
        std::cout << "Small font failed: " << TTF_GetError() << std::endl;

    normalFont =
        TTF_OpenFont(
            "Fonts/BasisGrotesqueArabicPro-Medium.ttf",
            18);

    if (!normalFont)
    {
        std::cout << TTF_GetError() << std::endl;
    }
    else
    {
        std::cout << "Normal font loaded!" << std::endl;
    }

    headingFont =
        TTF_OpenFont(
            "Fonts/BasisGrotesqueArabicPro-Bold.ttf",
            24);

    if (!headingFont)
        std::cout << "Small font failed: " << TTF_GetError() << std::endl;

    if (!smallFont ||
        !normalFont ||
        !headingFont)
    {
        shutdown();
        return false;
    }

    return true;
}

void FontManager::shutdown()
{
    if (smallFont)
    {
        TTF_CloseFont(smallFont);
        smallFont = nullptr;
    }

    if (normalFont)
    {
        TTF_CloseFont(normalFont);
        normalFont = nullptr;
    }

    if (headingFont)
    {
        TTF_CloseFont(headingFont);
        headingFont = nullptr;
    }

    TTF_Quit();
}

TTF_Font* FontManager::getSmallFont() const
{
    return smallFont;
}

TTF_Font* FontManager::getNormalFont() const
{
    return normalFont;
}

TTF_Font* FontManager::getHeadingFont() const
{
    return headingFont;
}