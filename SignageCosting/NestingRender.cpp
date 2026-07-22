#include "NestingRender.h"
#include <string>
#include <iostream>
#include "Renderer.h"

static bool printedDebug = false;

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

    static int debugCount = 0;
    debugCount++;
 
    if (!printedDebug)
    {
        printedDebug = true;

        std::cout
            << "\n========== DRAW SHEET DEBUG ==========\n"
            << "Sheet size: "
            << sheet.width
            << " x "
            << sheet.height
            << "\n"
            << "Placed items: "
            << sheet.placed.size()
            << "\n"
            << "Screen position: "
            << screenX
            << ", "
            << screenY
            << "\n"
            << "Scale: "
            << scale
            << "\n";
    }

    if (!printedDebug)
    {
        printedDebug = true;
    
        std::cout
            << "Scale: "
            << scale
            << "\n";
    }

    if (scale <= 0)
    {
        std::cout
            << "[ERROR] INVALID SCALE\n";

        return;
    }

    SDL_Rect sheetRect;
    sheetRect.x = screenX;
    sheetRect.y = screenY;
    sheetRect.w =
        sheet.width * scale;

    sheetRect.h =
        sheet.height * scale;

    if (!printedDebug)
    {
        printedDebug = true;
  
        std::cout
            << "Sheet Rect: "
            << sheetRect.x
            << ", "
            << sheetRect.y
            << " "
            << sheetRect.w
            << " x "
            << sheetRect.h
            << "\n";
    }

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

    int itemCount = 0;

    for (auto& r : sheet.placed)
    {

        SDL_Rect item;

        item.x =
            screenX +
            r.x * scale;

        item.y =
            screenY +
            r.y * scale;

        item.w =
            r.width * scale;

        item.h =
            r.height * scale;

        if (!printedDebug && itemCount < 3)
        {
            printedDebug = true;
            std::cout
                << "Item "
                << itemCount
                << ": "
                << item.x
                << ","
                << item.y
                << " "
                << item.w
                << "x"
                << item.h
                << "\n";
        }

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

        itemCount++;

    }

    if (!printedDebug)
    {
        printedDebug = true;

        std::cout
            << "Finished drawing sheet\n"
            << "====================================\n";
    }

}