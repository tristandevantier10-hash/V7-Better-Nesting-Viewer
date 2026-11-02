#include "NestingRender.h"
#include <iostream>
#include "Renderer.h"
#include <algorithm>
#include <cmath>

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
        if (r.contour)
        {
            renderer.pushClip(sheetRect);
            const auto box=NestingGeometry::bounds(*r.contour);
            for (int y=std::max(sheetRect.y,screenY+static_cast<int>(std::floor(box.top*scale)));
                y<std::min(sheetRect.y+sheetRect.h,screenY+static_cast<int>(std::ceil(box.bottom*scale)));++y)
            {
                const auto crossings=NestingGeometry::scanline(*r.contour,(y+0.5-screenY)/scale);
                for (size_t i=0;i+1<crossings.size();i+=2)
                {
                    const int left=screenX+static_cast<int>(std::ceil(crossings[i]*scale-0.5));
                    const int right=screenX+static_cast<int>(std::ceil(crossings[i+1]*scale-0.5))-1;
                    if (left<=right) renderer.drawLine(left,y,right,y,{50,120,220,255});
                }
            }
            for (const auto* boundary:NestingGeometry::boundaryRings(*r.contour))
                for (size_t i=0;i<boundary->vertices.size();++i)
                {
                    const auto a=boundary->vertices[i],b=boundary->vertices[(i+1)%boundary->vertices.size()];
                    renderer.drawLine(screenX+static_cast<int>(std::round(a.x*scale)),screenY+static_cast<int>(std::round(a.y*scale)),
                        screenX+static_cast<int>(std::round(b.x*scale)),screenY+static_cast<int>(std::round(b.y*scale)),{0,0,0,255});
                }
            renderer.popClip();
            continue;
        }

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
