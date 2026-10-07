#include "Renderer.h"
#include "TextRenderer.h"
#include "Label.h"
#include <algorithm>
#include <cmath>
#include <lunasvg/lunasvg.h>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <iterator>

Renderer::Renderer(
    SDL_Renderer* renderer,
    FontManager& fonts,
    Theme& theme)
    :
    sdlRenderer(renderer),
    fontManager(fonts),
    theme(theme),
    shapeRenderer(renderer)

{}

void Renderer::fillRect(
    const SDL_Rect& rect,
    SDL_Color colour)
{
    SDL_Rect translated = rect;

    translated.x += offsetX;
    translated.y += offsetY;

    shapeRenderer.fillRect(
        translated,
        colour);
}

void Renderer::fillRoundedRect(
    const SDL_Rect& rect,
    SDL_Color colour,
    int radius)
{
    SDL_Rect translated = rect;

    translated.x += offsetX;
    translated.y += offsetY;

    shapeRenderer.fillRoundedRect(
        translated,
        colour,
        radius);
}

void Renderer::fillCircle(
    int cx,
    int cy,
    int radius,
    SDL_Color colour)
{
    shapeRenderer.fillCircle(
        cx + offsetX,
        cy + offsetY,
        radius,
        colour);
}

void Renderer::drawRect(
    const SDL_Rect& rect,
    SDL_Color colour)
{
    SDL_Rect translated = rect;

    translated.x += offsetX;
    translated.y += offsetY;

    shapeRenderer.drawRect(
        translated,
        colour);
}

FontManager& Renderer::getFontManager()
{
    return fontManager;
}

void Renderer::drawText(
    const std::string& text,
    int x,
    int y)
{
    TextRenderer::drawText(
        sdlRenderer,
        text,
        x + offsetX,
        y + offsetY,
        theme.darkText
    );
}

void Renderer::drawText(
    const std::string& text,
    int x,
    int y,
    SDL_Color colour)
{
    TextRenderer::drawText(
        sdlRenderer,
        text,
        x + offsetX,
        y + offsetY,
        colour
    );
}

void Renderer::drawText(
    const std::string& text,
    int x,
    int y,
    TTF_Font* font,
    SDL_Color colour)
{
    if (font == nullptr)
        font = fontManager.getNormalFont();

    TextRenderer::drawText(
        sdlRenderer,
        font,
        text,
        x + offsetX,
        y + offsetY,
        colour);
}

void Renderer::drawText(
    const std::string& text,
    int x,
    int y,
    LabelStyle style,
    SDL_Color colour)
{
    TTF_Font* font = nullptr;

    switch (style)
    {
    case LabelStyle::Small:
        font = fontManager.getSmallFont();
        break;

    case LabelStyle::Heading:
        font = fontManager.getHeadingFont();
        break;

    default:
        font = fontManager.getNormalFont();
        break;
    }

    TextRenderer::drawText(
        sdlRenderer,
        font,
        text,
        x + offsetX,
        y + offsetY,
        colour);
}

void Renderer::drawSVG(
    const std::string& path,
    const SDL_Rect& destination,
    SDL_Color colour)
{
    if (path.empty())
        return;

    std::filesystem::path svgPath =
        std::filesystem::absolute(path);

    if (!std::filesystem::exists(svgPath))
        return;

    std::ifstream file(svgPath);

    if (!file)
    {
        return;
    }

    std::string svgData(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    auto document =
        lunasvg::Document::loadFromData(
            svgData);

    if (!document)
        return;

    lunasvg::Bitmap bitmap =
        document->renderToBitmap(
            destination.w,
            destination.h);

    if (bitmap.width() <= 0 ||
        bitmap.height() <= 0)
    {
        return;
    }

    SDL_Surface* surface =
        SDL_CreateRGBSurfaceFrom(
            bitmap.data(),
            bitmap.width(),
            bitmap.height(),
            32,
            bitmap.stride(),
            0x000000FF,
            0x0000FF00,
            0x00FF0000,
            0xFF000000);

    if (!surface)
        return;

    SDL_Texture* texture =
        SDL_CreateTextureFromSurface(
            sdlRenderer,
            surface);

    SDL_FreeSurface(surface);

    if (!texture)
        return;

    SDL_SetTextureBlendMode(
        texture,
        SDL_BLENDMODE_BLEND);

    SDL_SetTextureColorMod(
        texture,
        colour.r,
        colour.g,
        colour.b);

    SDL_SetTextureAlphaMod(
        texture,
        colour.a);

    SDL_RenderCopy(
        sdlRenderer,
        texture,
        nullptr,
        &destination);

    SDL_DestroyTexture(texture);
}

void Renderer::drawLine(
    int x1,
    int y1,
    int x2,
    int y2,
    SDL_Color colour)
{
    shapeRenderer.drawLine(
        x1 + offsetX,
        y1 + offsetY,
        x2 + offsetX,
        y2 + offsetY,
        colour);
}

void Renderer::beginFrame()
{
    SDL_SetRenderDrawColor(
        sdlRenderer,
        theme.windowBackground.r,
        theme.windowBackground.g,
        theme.windowBackground.b,
        theme.windowBackground.a);

    SDL_RenderClear(sdlRenderer);
}

void Renderer::endFrame()
{
    SDL_RenderPresent(sdlRenderer);
}

SDL_Renderer* Renderer::getSDLRenderer()
{
    return sdlRenderer;
}

void Renderer::pushOffset(int x, int y)
{
    offsetStack.push_back({ offsetX, offsetY });

    offsetX += x;
    offsetY += y;
}

void Renderer::popOffset()
{
    if (offsetStack.empty())
        return;

    SDL_Point p = offsetStack.back();

    offsetStack.pop_back();

    offsetX = p.x;
    offsetY = p.y;
}

void Renderer::pushClip(const SDL_Rect& rect)
{
    SDL_Rect translated = rect;

    translated.x += offsetX;
    translated.y += offsetY;

    if (!clipStack.empty())
    {
        SDL_Rect parent = clipStack.back();

        int left =
            std::max(
                translated.x,
                parent.x);

        int top =
            std::max(
                translated.y,
                parent.y);

        int right =
            std::min(
                translated.x + translated.w,
                parent.x + parent.w);

        int bottom =
            std::min(
                translated.y + translated.h,
                parent.y + parent.h);

        translated.x = left;
        translated.y = top;
        translated.w = std::max(0, right - left);
        translated.h = std::max(0, bottom - top);
    }

    clipStack.push_back(translated);

    SDL_RenderSetClipRect(
        sdlRenderer,
        &clipStack.back());
}

void Renderer::popClip()
{
    if (!clipStack.empty())
    {
        clipStack.pop_back();
    }

    if (clipStack.empty())
    {
        SDL_RenderSetClipRect(
            sdlRenderer,
            nullptr);
    }
    else
    {
        SDL_RenderSetClipRect(
            sdlRenderer,
            &clipStack.back());
    }
}

int Renderer::getTextWidth(const std::string& text)
{
    return TextRenderer::getTextWidth(text);
}

void Renderer::drawRoundedRect(
    const SDL_Rect& rect,
    SDL_Color colour,
    int radius)
{
    SDL_Rect translated = rect;

    translated.x += offsetX;
    translated.y += offsetY;

    shapeRenderer.drawRoundedRect(
        translated,
        colour,
        radius);
}

void Renderer::drawRoundedRing(
    const SDL_Rect& rect,
    SDL_Color colour,
    int radius,
    int thickness)
{
    SDL_Rect translated = rect;

    translated.x += offsetX;
    translated.y += offsetY;

    shapeRenderer.drawRoundedRing(
        translated,
        colour,
        radius,
        thickness);
}