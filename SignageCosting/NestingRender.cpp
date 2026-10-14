#include "NestingRender.h"
#include <iostream>
#include "Renderer.h"

void NestingRenderer::drawSheet(
    Renderer& renderer,
    const Sheet& sheet,
    int screenX,
    int screenY,
    float scale
)
{
    SDL_Renderer* sdl =
        renderer.getSDLRenderer();

    if (scale <= 0)
    {
        std::cerr << "Invalid nesting preview scale.\n";

        return;
    }

    SDL_Rect sheetRect;
    sheetRect.x = screenX;
    sheetRect.y = screenY;
    sheetRect.w = static_cast<int>(
        sheet.width * scale);

    sheetRect.h = static_cast<int>(
        sheet.height * scale);

    // sheet background

    SDL_SetRenderDrawColor(
        sdl,
        220,
        220,
        220,
        255
    );

    SDL_RenderFillRect(
        sdl,
        &sheetRect
    );

    // border

    SDL_SetRenderDrawColor(
        sdl,
        0,
        0,
        0,
        255
    );

    SDL_RenderDrawRect(
        sdl,
        &sheetRect
    );

    // draw placed rectangles

    for (auto& r : sheet.placed)
    {

        SDL_Rect item;

        item.x = static_cast<int>(
            screenX + r.x * scale);

        item.y = static_cast<int>(
            screenY + r.y * scale);

        item.w = static_cast<int>(
            r.width * scale);

        item.h = static_cast<int>(
            r.height * scale);

        SDL_SetRenderDrawColor(
            sdl,
            50,
            120,
            220,
            255
        );

        SDL_RenderFillRect(
            sdl,
            &item
        );

        SDL_SetRenderDrawColor(
            sdl,
            0,
            0,
            0,
            255
        );

        SDL_RenderDrawRect(
            sdl,
            &item
        );

    }
}
