#include "TextRenderer.h"
#include <SDL2/SDL_ttf.h>
#include "FontManager.h"
#include <iostream>

FontManager* TextRenderer::fontManager = nullptr;

bool TextRenderer::initialise(FontManager* fonts)
{
    fontManager = fonts;
    return fontManager != nullptr;
}

void TextRenderer::shutdown()
{
    fontManager = nullptr;
}

void TextRenderer::drawText(
    SDL_Renderer* renderer,
    const std::string& text,
    int x,
    int y,
    SDL_Color colour)

{

    if (text.empty())
        return;

    /*std::cout << "Drawing: " << text << std::endl;*/
    if (!fontManager)
        return;

    TTF_Font* font = fontManager->getNormalFont();

    if (!font)
        return;

    SDL_Surface* surface =
        TTF_RenderUTF8_Blended(
            font,
            text.c_str(),
            colour);

    if (!surface)
    {
        std::cout << TTF_GetError() << std::endl;
        return;
    }

    /*std::cout
        << "Surface "
        << surface->w
        << " x "
        << surface->h
        << std::endl;
        */

    SDL_Texture* texture =
        SDL_CreateTextureFromSurface(
            renderer,
            surface);

    if (!texture)
    {
        std::cout << SDL_GetError() << std::endl;
        return;
    }

    SDL_Rect dst;

    dst.x = x;
    dst.y = y;
    dst.w = surface->w;
    dst.h = surface->h;

    SDL_FreeSurface(surface);

    /*std::cout
        << text
        << " at "
        << dst.x
        << ", "
        << dst.y
        << std::endl;
        */

    SDL_RenderCopy(
        renderer,
        texture,
        nullptr,
        &dst);

    SDL_DestroyTexture(texture);
}



int TextRenderer::getTextWidth(const std::string& text)
{
    if (!fontManager)
        return 0;

    TTF_Font* font = fontManager->getNormalFont();

    if (!font)
        return 0;

    int w = 0;
    int h = 0;

    TTF_SizeUTF8(
        font,
        text.c_str(),
        &w,
        &h);

    return w;
}