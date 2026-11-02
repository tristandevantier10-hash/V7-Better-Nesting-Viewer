#define SDL_MAIN_HANDLED
#include "../NestingPreviewPanel.h"
#include "../NestingRender.h"
#include <iostream>
#include <stdexcept>

using namespace NestingGeometry;
void check(bool value,const char* message) { if (!value) throw std::runtime_error(message); }
SDL_Color pixel(SDL_Renderer* sdl,int x,int y)
{
    Uint32 value=0;
    SDL_Rect sample{x,y,1,1};
    check(SDL_RenderReadPixels(sdl,&sample,SDL_PIXELFORMAT_RGBA32,&value,4)==0,"Read software-rendered pixel.");
    auto* format=SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
    SDL_Color colour{};
    SDL_GetRGBA(value,format,&colour.r,&colour.g,&colour.b,&colour.a);
    SDL_FreeFormat(format);
    return colour;
}
bool equal(SDL_Color a,SDL_Color b) { return a.r==b.r && a.g==b.g && a.b==b.b; }
int main()
{
    SDL_SetMainReady(); SDL_Init(0);
    try
    {
        auto* surface=SDL_CreateRGBSurfaceWithFormat(0,800,600,32,SDL_PIXELFORMAT_RGBA32);
        check(surface!=nullptr,"Create preview surface.");
        auto* sdl=SDL_CreateSoftwareRenderer(surface);
        check(sdl!=nullptr,"Create software renderer.");
        FontManager fonts;
        check(fonts.initialise(),"Load preview fonts (run from SignageCosting).");
        Theme theme{};
        {
            Renderer renderer(sdl,fonts,theme);
            Polygon frame=rectangle(100,100);
            frame.holes={transformed(rectangle(60,60),{20,20,0})};
            const Polygon plug=transformed(rectangle(20,20),{40,40,0});
            Sheet sheet; sheet.width=100; sheet.height=100;
            PlacedRect outer{0,0,100,100,false,1,0}; outer.contour=std::make_shared<const Polygon>(frame);
            PlacedRect insert{40,40,20,20,false,2,0}; insert.contour=std::make_shared<const Polygon>(plug);
            NestingPreviewPanel panel;
            panel.setPosition(0,0); panel.setSize(800,600);
            panel.setGeometryParts({{1,frame,1,{0},"Frame"},{2,rectangle(20,20),1,{0},"Insert"}});
            for (bool reverse:{false,true})
            {
                sheet.placed=reverse ? std::vector<PlacedRect>{insert,outer} : std::vector<PlacedRect>{outer,insert};
                panel.setSheets({sheet}); panel.render(renderer);
                // First paper is (27,118), 354 pixels square with this panel.
                check(equal(pixel(sdl,62,153),NestingStyle::colours[0]),"Frame material is filled.");
                check(equal(pixel(sdl,133,224),{220,222,223,255}),"Unoccupied cutout stays paper coloured.");
                check(equal(pixel(sdl,204,295),NestingStyle::colours[1]),"Nested part survives either drawing order.");
                NestingRenderer::drawSheet(renderer,sheet,0,0,4);
                check(equal(pixel(sdl,40,40),{50,120,220,255}),"Legacy sheet preview fills actual material.");
                check(equal(pixel(sdl,120,120),{220,220,220,255}),"Legacy sheet preview also leaves cutouts empty.");
                check(equal(pixel(sdl,200,200),{50,120,220,255}),"Legacy preview preserves a nested part.");
            }
        }
        fonts.shutdown();
        SDL_DestroyRenderer(sdl); SDL_FreeSurface(surface); SDL_Quit();
        std::cout<<"Preview checks passed: empty cutouts and nested parts in either drawing order.\n";
        return 0;
    }
    catch (const std::exception& error) { std::cerr<<error.what()<<'\n'; return 1; }
}
