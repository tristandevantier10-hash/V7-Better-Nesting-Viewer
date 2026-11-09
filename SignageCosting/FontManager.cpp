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
        std::cerr << "Could not load small font: " << TTF_GetError() << std::endl;

    normalFont =
        TTF_OpenFont(
            "Fonts/BasisGrotesqueArabicPro-Medium.ttf",
            18);

    if (!normalFont)
    {
        std::cerr << "Could not load normal font: " << TTF_GetError() << std::endl;
    }

    headingFont =
        TTF_OpenFont(
            "Fonts/BasisGrotesqueArabicPro-Bold.ttf",
            24);

    pageTitleFont =
        TTF_OpenFont(
            "Fonts/BasisGrotesqueArabicPro-Bold.ttf",
            32);
    sectionFont = TTF_OpenFont("Fonts/BasisGrotesqueArabicPro-Bold.ttf", 16);

    headerSubtitleFont = TTF_OpenFont("Fonts/BasisGrotesqueArabicPro-Regular.ttf", 16);
    brandBlackFont = TTF_OpenFont("Fonts/BasisGrotesqueArabicPro-Black.ttf", 11);
    brandLightFont = TTF_OpenFont("Fonts/BasisGrotesqueArabicPro-Light.ttf", 11);
    navigationFont = TTF_OpenFont("Fonts/BasisGrotesqueArabicPro-Light.ttf", 15);

    if (!headingFont)
        std::cerr << "Could not load heading font: " << TTF_GetError() << std::endl;

    if (!pageTitleFont)
        std::cerr << "Could not load page title font: " << TTF_GetError() << std::endl;

    if (!smallFont ||
        !normalFont ||
        !headingFont ||
        !pageTitleFont)
    {
        shutdown();
        return false;
    }

    return true;
}

void FontManager::shutdown()
{
    if (headerSubtitleFont)
    {
        TTF_CloseFont(headerSubtitleFont);
        headerSubtitleFont = nullptr;
    }

    if (brandBlackFont)
    {
        TTF_CloseFont(brandBlackFont);
        brandBlackFont = nullptr;
    }

    if (navigationFont)
    {
        TTF_CloseFont(navigationFont);
        navigationFont = nullptr;
    }

    if (brandLightFont)
    {
        TTF_CloseFont(brandLightFont);
        brandLightFont = nullptr;
    }

    if (sectionFont)
    {
        TTF_CloseFont(sectionFont);
        sectionFont = nullptr;
    }

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

    if (pageTitleFont)
    {
        TTF_CloseFont(pageTitleFont);
        pageTitleFont = nullptr;
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

TTF_Font* FontManager::getPageTitleFont() const
{
    return pageTitleFont;
}

TTF_Font* FontManager::getSectionFont() const
{
    return sectionFont ? sectionFont : normalFont;
}
