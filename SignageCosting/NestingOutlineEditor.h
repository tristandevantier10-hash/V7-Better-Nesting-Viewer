#pragma once
#include "NestingStyle.h"
#include "NestingGeometry.h"
#include <functional>
#include <algorithm>
#include <cmath>

class NestingOutlineEditor : public UIElement
{
public:
    std::function<void(const NestingGeometry::Polygon&,const std::string&)> accepted;
    std::function<void()> cancelled;
    void begin(const NestingGeometry::Polygon& outline,const std::string& label,double width,double height)
    {
        polygon=outline; name=label; originalAspect=std::clamp(width/height,0.1,10.0); aspect=label=="Circle" ? 1 : originalAspect;
        message.clear(); drawing=false; setVisible(true);
    }
    void update(const SDL_Event& event) override
    {
        if (!visible) return;
        arrange();
        if (event.type==SDL_KEYDOWN)
        {
            if (event.key.keysym.sym==SDLK_ESCAPE) { if (cancelled) cancelled(); }
            if (event.key.keysym.sym==SDLK_BACKSPACE) undo();
            if (event.key.keysym.sym==SDLK_RETURN) finish();
        }
        if (event.type!=SDL_MOUSEBUTTONDOWN || event.button.button!=SDL_BUTTON_LEFT) return;
        SDL_Point point{event.button.x,event.button.y};
        for (int index=0;index<4;++index)
            if (SDL_PointInRect(&point,&presets[index]))
            {
                aspect=originalAspect;
                if (index==0) { polygon=NestingGeometry::rectangle(1,1); name="Rectangle"; }
                if (index==1) { polygon={{{0,0},{1,0},{0,1}}}; name="Triangle"; }
                if (index==2) { polygon={{{0,0},{1,0},{1,0.4},{0.4,0.4},{0.4,1},{0,1}}}; name="L-shape"; }
                if (index==3) { polygon=NestingGeometry::circle(1); name="Circle"; aspect=1; }
                drawing=false; message.clear(); return;
            }
        if (SDL_PointInRect(&point,&undoButton)) { undo(); return; }
        if (SDL_PointInRect(&point,&clearButton)) { polygon={}; drawing=true; name="Custom outline"; message.clear(); return; }
        if (SDL_PointInRect(&point,&cancelButton)) { if (cancelled) cancelled(); return; }
        if (SDL_PointInRect(&point,&useButton)) { finish(); return; }
        if (SDL_PointInRect(&point,&canvas))
        {
            if (!drawing) { polygon={}; drawing=true; name="Custom outline"; }
            if (polygon.vertices.size()>=64) { message="Use up to 64 points per outline."; return; }
            // A 20-division grid makes straight edges easy to draw consistently.
            const double x=std::clamp(std::round((point.x-canvas.x)*20.0/canvas.w)/20.0,0.0,1.0);
            const double y=std::clamp(std::round((point.y-canvas.y)*20.0/canvas.h)/20.0,0.0,1.0);
            if (polygon.vertices.empty() || std::hypot(x-polygon.vertices.back().x,y-polygon.vertices.back().y)>1e-7)
                polygon.vertices.push_back({x,y});
            message.clear();
        }
    }
    void render(Renderer& renderer) override
    {
        if (!visible) return;
        using namespace NestingStyle;
        arrange();
        SDL_BlendMode previousBlend;
        SDL_GetRenderDrawBlendMode(renderer.getSDLRenderer(),&previousBlend);
        SDL_SetRenderDrawBlendMode(renderer.getSDLRenderer(),SDL_BLENDMODE_BLEND);
        renderer.fillRect(bounds,{18,26,36,155});
        SDL_SetRenderDrawBlendMode(renderer.getSDLRenderer(),previousBlend);
        card(renderer,popup);
        text(renderer,"Edit outline",popup.x+20,popup.y+18,LabelStyle::Heading);
        text(renderer,"Choose a preset, or click the grid to draw a new outline.",popup.x+20,popup.y+54,LabelStyle::Small,muted);
        text(renderer,"The outline is scaled to this row's width and height.",popup.x+20,popup.y+75,LabelStyle::Small,muted);
        auto button=[&](SDL_Rect rect,const std::string& label,bool primary=false)
        {
            renderer.fillRoundedRect(rect,primary ? green : white,6);
            renderer.drawRoundedRect(rect,primary ? green : border,6);
            int w=0,h=0; auto* font=renderer.getFontManager().getSmallFont();
            TTF_SizeUTF8(font,label.c_str(),&w,&h);
            renderer.drawText(label,rect.x+(rect.w-w)/2,rect.y+(rect.h-h)/2,font,primary ? white : ink);
        };
        for (int index=0;index<4;++index) button(presets[index],std::array<const char*,4>{"Rectangle","Triangle","L-shape","Circle"}[index]);
        renderer.fillRect(canvas,{249,251,253,255});
        for (int step=0;step<=20;++step)
        {
            const int x=canvas.x+step*canvas.w/20,y=canvas.y+step*canvas.h/20;
            renderer.drawLine(x,canvas.y,x,canvas.y+canvas.h,border);
            renderer.drawLine(canvas.x,y,canvas.x+canvas.w,y,border);
        }
        renderer.drawRect(canvas,border);
        for (size_t i=0;i<polygon.vertices.size();++i)
        {
            const auto& a=polygon.vertices[i];
            const int x=canvas.x+static_cast<int>(a.x*canvas.w), y=canvas.y+static_cast<int>(a.y*canvas.h);
            renderer.fillCircle(x,y,4,green);
            if (i>0)
            {
                const auto& b=polygon.vertices[i-1];
                renderer.drawLine(canvas.x+static_cast<int>(b.x*canvas.w),canvas.y+static_cast<int>(b.y*canvas.h),x,y,green);
            }
        }
        if (polygon.vertices.size()>=3)
        {
            const auto& a=polygon.vertices.front(); const auto& b=polygon.vertices.back();
            renderer.drawLine(canvas.x+static_cast<int>(a.x*canvas.w),canvas.y+static_cast<int>(a.y*canvas.h),
                canvas.x+static_cast<int>(b.x*canvas.w),canvas.y+static_cast<int>(b.y*canvas.h),green);
        }
        for (const auto& hole:polygon.holes)
            for (size_t i=0;i<hole.vertices.size();++i)
            {
                const auto a=hole.vertices[i],b=hole.vertices[(i+1)%hole.vertices.size()];
                renderer.drawLine(canvas.x+static_cast<int>(a.x*canvas.w),canvas.y+static_cast<int>(a.y*canvas.h),
                    canvas.x+static_cast<int>(b.x*canvas.w),canvas.y+static_cast<int>(b.y*canvas.h),green);
            }
        button(undoButton,"Undo"); button(clearButton,"Clear");
        button(cancelButton,"Cancel"); button(useButton,"Use outline",true);
        renderer.pushClip({popup.x+20,popup.y+popup.h-94,popup.w-40,30});
        text(renderer,message.empty() ? std::to_string(polygon.vertices.size())+" points · "+std::to_string(polygon.holes.size())+" cutouts" : message,
            popup.x+20,popup.y+popup.h-92,LabelStyle::Small,message.empty() ? muted : SDL_Color{174,64,46,255});
        renderer.popClip();
    }
private:
    void arrange()
    {
        const int w=std::min(720,bounds.w-40),h=std::min(640,bounds.h-40);
        popup={bounds.x+(bounds.w-w)/2,bounds.y+(bounds.h-h)/2,w,h};
        for (int i=0;i<4;++i) presets[i]={popup.x+20+i*124,popup.y+104,114,32};
        const int availableW=w-60,availableH=h-260;
        const int canvasW=std::min(availableW,static_cast<int>(availableH*aspect));
        const int canvasH=std::min(availableH,static_cast<int>(canvasW/aspect));
        canvas={popup.x+(w-canvasW)/2,popup.y+150,canvasW,canvasH};
        undoButton={popup.x+20,popup.y+h-54,80,34}; clearButton={popup.x+110,popup.y+h-54,80,34};
        cancelButton={popup.x+w-242,popup.y+h-54,100,34}; useButton={popup.x+w-132,popup.y+h-54,112,34};
    }
    void undo()
    {
        if (!polygon.vertices.empty()) polygon.vertices.pop_back();
        drawing=true; name="Custom outline"; message.clear();
    }
    void finish()
    {
        const auto errors=NestingGeometry::validateOutline(polygon);
        if (!errors.empty()) { message=errors.front(); return; }
        const auto box=NestingGeometry::bounds(polygon);
        auto normalized=polygon;
        NestingGeometry::mapVertices(normalized,[&](NestingGeometry::Point p)
        { return NestingGeometry::Point{(p.x-box.left)/(box.right-box.left),(p.y-box.top)/(box.bottom-box.top)}; });
        if (accepted) accepted(normalized,name);
    }
    NestingGeometry::Polygon polygon;
    std::string name,message;
    bool drawing=false;
    double aspect=1,originalAspect=1;
    SDL_Rect popup{},canvas{},presets[4]{},undoButton{},clearButton{},cancelButton{},useButton{};
};
