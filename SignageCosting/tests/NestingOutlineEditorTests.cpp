#define SDL_MAIN_HANDLED
#include "../NestingOutlineEditor.h"
#include <iostream>
#include <stdexcept>

void check(bool value,const char* message) { if (!value) throw std::runtime_error(message); }
int main()
{
    SDL_SetMainReady(); SDL_Init(0);
    try
    {
        NestingOutlineEditor editor;
        editor.setPosition(0,0); editor.setSize(1280,1000);
        int accepted=0,cancelled=0;
        NestingGeometry::Polygon saved;
        editor.accepted=[&](const NestingGeometry::Polygon& polygon,const std::string&)
        { saved=polygon; ++accepted; editor.setVisible(false); };
        editor.cancelled=[&]() { ++cancelled; editor.setVisible(false); };
        auto click=[&](int x,int y)
        {
            SDL_Event event{}; event.type=SDL_MOUSEBUTTONDOWN;
            event.button.button=SDL_BUTTON_LEFT; event.button.x=x; event.button.y=y;
            editor.update(event);
        };
        editor.begin(NestingGeometry::rectangle(1,1),"Rectangle",100,100);
        click(480,300); // Triangle preset.
        click(920,783);
        check(accepted==1 && std::abs(NestingGeometry::area(saved)-0.5)<1e-7,"Triangle preset must be accepted.");
        editor.begin(saved,"Triangle",100,100);
        click(600,300); // L-shape preset.
        click(920,783);
        check(accepted==2 && std::abs(NestingGeometry::area(saved)-0.64)<1e-7,"L-shape preset must preserve recess.");
        editor.begin(saved,"L-shape",100,100);
        click(450,330); click(829,330);
        click(920,783);
        check(accepted==2 && editor.isVisible(),"Incomplete drawing must not be accepted.");
        click(829,482); click(602,482); click(602,709); click(450,709);
        click(920,783);
        check(accepted==3 && std::abs(NestingGeometry::area(saved)-0.64)<1e-7,"Drawn snapped outline must be normalized and accepted.");
        editor.begin(saved,"Custom outline",100,100);
        click(450,330); click(829,709); click(450,709); click(829,330);
        click(920,783);
        check(accepted==3 && editor.isVisible(),"Self-crossing drawing must remain editable.");
        SDL_Event escape{}; escape.type=SDL_KEYDOWN; escape.key.keysym.sym=SDLK_ESCAPE;
        editor.update(escape);
        check(cancelled==1 && !editor.isVisible(),"Escape cancels without committing invalid geometry.");
        editor.begin(NestingGeometry::rectangle(1,1),"Rectangle",600,400);
        click(720,300); click(920,783);
        check(accepted==4 && saved.vertices.size()==64,"Circle preset must be accepted.");
        std::cout<<"Outline editor checks passed: presets, drawing, normalization, incomplete/crossing outlines and cancellation.\n";
        return 0;
    }
    catch (const std::exception& error) { std::cerr<<error.what()<<'\n'; return 1; }
}
