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
    TTF_Font* getPageTitleFont() const;
    TTF_Font* getSectionFont() const;
    TTF_Font* getHeaderSubtitleFont() const { return headerSubtitleFont ? headerSubtitleFont : normalFont; }
    TTF_Font* getBrandBlackFont() const { return brandBlackFont; }
    TTF_Font* getNavigationFont() const { return navigationFont ? navigationFont : smallFont; }
    TTF_Font* getBrandLightFont() const { return brandLightFont; }

private:

    TTF_Font* smallFont = nullptr;

    TTF_Font* normalFont = nullptr;

    TTF_Font* headingFont = nullptr;
    TTF_Font* pageTitleFont = nullptr;
    TTF_Font* sectionFont = nullptr;
    TTF_Font* headerSubtitleFont = nullptr;
    TTF_Font* brandBlackFont = nullptr;
    TTF_Font* brandLightFont = nullptr;
    TTF_Font* navigationFont = nullptr;
};
