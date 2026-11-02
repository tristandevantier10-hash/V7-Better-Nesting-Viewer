#include "../NestingService.h"
#include "../NestingGeometryBridge.h"
#include <iostream>
#include <stdexcept>

using namespace NestingGeometry;
void check(bool condition,const char* message) { if (!condition) throw std::runtime_error(message); }
void verify(const std::vector<Part>& parts,const NestingService::Result& result,
    const NestingService::Options& options)
{
    check(result.layout.isValid() && !result.layout.sheets.empty(),"Expected a complete valid nest.");
    check(validateLayout(parts,contourLayouts(result.layout.sheets),options.clearance,options.margin).empty(),
        "Output must pass independent contour, quantity, rotation and identity checks.");
}
int main()
{
    try
    {
        const std::vector<double> quarterTurns{0,90,180,270};
        std::vector<Part> rectangles{{101,rectangle(500,500),50,{0,90}},{102,rectangle(300,100),34,{0,90}}};
        NestingService::Options large{2440,1220};
        auto standard=NestingService::calculate(rectangles,large);
        check(standard.method==Method::Rectangles,"Rectangles must use the existing fast method.");
        check(standard.layout.sheets.size()==7 && standard.layout.sheets.front().placed.size()==37,
            "Preserve the full-job rectangle regression.");
        verify(rectangles,standard,large);

        Polygon triangle{{{0,0},{100,0},{0,100}}};
        std::vector<Part> triangles{{201,triangle,2,quarterTurns,"Triangle"}};
        NestingService::Options small{100,100};
        auto interlocked=NestingService::calculate(triangles,small);
        check(interlocked.method==Method::Contours,"Custom outlines must use contour placement.");
        check(interlocked.layout.sheets.size()==1,"Two complementary triangles must share one sheet.");
        check(interlocked.layout.sheets.front().placed.size()==2,"Preserve both triangle copies.");
        check(interlocked.layout.sheets.front().placed.front().contour!=nullptr,"Preview must receive real outlines.");
        verify(triangles,interlocked,small);

        // Exact reported case: ten 500 mm triangles should form five full
        // squares when rotation is allowed, not ten separate half-squares.
        std::vector<Part> tenTriangles{{211,{{{0,0},{500,0},{0,500}}},10,quarterTurns,"Triangle"}};
        NestingService::Options wide{3000,1500};
        auto pairs=NestingService::calculate(tenTriangles,wide);
        verify(tenTriangles,pairs,wide);
        check(pairs.layout.sheets.size()==1 && pairs.layout.sheets.front().placed.size()==10,"Preserve all ten triangles.");
        double right=0,bottom=0;
        for (const auto& part:pairs.layout.sheets.front().placed)
        { right=std::max(right,part.x+part.width); bottom=std::max(bottom,part.y+part.height); }
        check(right*bottom<=1250000.0001,"Ten triangles must occupy five complete squares without internal waste.");

        Polygon elbow{{{0,0},{100,0},{100,40},{40,40},{40,100},{0,100}}};
        std::vector<Part> mixed{{301,elbow,1,quarterTurns,"L-shape"},{302,rectangle(60,60),1,quarterTurns,"Rectangle"}};
        auto notch=NestingService::calculate(mixed,small);
        check(notch.method==Method::Contours && notch.layout.sheets.size()==1,
            "A rectangle must fit inside the concave part's empty recess.");
        verify(mixed,notch,small);

        Polygon frame=rectangle(100,100);
        frame.holes={transformed(rectangle(60,60),{20,20,0})};
        std::vector<Part> cutouts{{701,frame,1,quarterTurns,"Frame"},{702,rectangle(60,60),1,quarterTurns,"Insert"}};
        auto filled=NestingService::calculate(cutouts,small);
        verify(cutouts,filled,small);
        check(filled.method==Method::Contours && filled.layout.sheets.size()==1,"Insert uses cutout rather than another sheet.");
        check(filled.layout.sheets[0].placed[0].contour->holes.size()==1,"Preview retains cutout rings.");
        double material=0;
        for (const auto& placed:filled.layout.sheets[0].placed) material+=area(*placed.contour);
        check(std::abs(material-10000)<1e-6,"A filled frame uses exactly one sheet's area without double counting.");
        cutouts[1].outline=rectangle(56,56);
        NestingService::Options cutoutGap{100,100,2,0};
        auto spacedCutout=NestingService::calculate(cutouts,cutoutGap);
        verify(cutouts,spacedCutout,cutoutGap);
        check(spacedCutout.layout.sheets.size()==1,"Search offsets both axes for cutout clearance.");
        cutoutGap.clearance=2.1;
        auto tooTight=NestingService::calculate(cutouts,cutoutGap);
        verify(cutouts,tooTight,cutoutGap);
        check(tooTight.layout.sheets.size()==2,"Clearance prevents an oversized insert using a hole.");
        Polygon wideFrame=rectangle(100,60);
        wideFrame.holes={transformed(rectangle(70,30),{15,15,0})};
        std::vector<Part> rotatedCutouts{{711,wideFrame,1,{90},"Rotated frame"},{712,rectangle(30,70),1,{0},"Insert"}};
        auto rotatedHole=NestingService::calculate(rotatedCutouts,{60,100});
        verify(rotatedCutouts,rotatedHole,{60,100});
        check(rotatedHole.layout.sheets.size()==1,"Cutout candidates follow parent rotation.");
        Polygon multiple=rectangle(100,100);
        multiple.holes={transformed(rectangle(30,60),{10,20,0}),transformed(rectangle(30,60),{60,20,0})};
        std::vector<Part> multiParts{{721,multiple,1,{0},"Two cutouts"},{722,rectangle(30,60),2,{0},"Insert"}};
        auto multiNest=NestingService::calculate(multiParts,small);
        verify(multiParts,multiNest,small);
        check(multiNest.layout.sheets.size()==1,"Both cutouts are reusable nesting space.");

        triangles.front().allowedRotations={0};
        auto fixed=NestingService::calculate(triangles,small);
        check(fixed.layout.sheets.size()==2,"Fixed orientation must not be overridden to save a sheet.");
        verify(triangles,fixed,small);
        std::vector<Part> fixedRectangle{{401,rectangle(60,40),1,{0}}};
        auto cannotRotate=NestingService::calculate(fixedRectangle,{40,60});
        check(!cannotRotate.layout.isValid(),"Do not silently rotate a fixed rectangle.");

        std::vector<Part> spaced{{501,rectangle(30,20),4,{0},"Rectangle"}};
        NestingService::Options withGaps{100,100,2,3};
        auto gaps=NestingService::calculate(spaced,withGaps);
        verify(spaced,gaps,withGaps);
        auto translated=NestingService::calculate({{601,transformed(rectangle(20,40),{-8,9,0}),1,{0,90}}},{40,20});
        verify({{601,transformed(rectangle(20,40),{-8,9,0}),1,{0,90}}},translated,{40,20});

        check(!NestingService::calculate({},small).layout.isValid(),"Reject empty jobs.");
        check(!NestingService::calculate({{1,{},1,{0}}},small).layout.isValid(),"Reject malformed outlines.");
        check(!NestingService::calculate({{1,triangle,1,{0}},{1,triangle,1,{0}}},small).layout.isValid(),"Reject duplicate identities.");
        check(!NestingService::calculate({{1,triangle,201,{0}}},small).layout.isValid(),"Reject over-budget contour requests clearly.");
        check(!NestingService::calculate(mixed,{0,100}).layout.isValid(),"Reject invalid sheets.");
        std::cout << "Service checks passed: automatic dispatch, rectangle compatibility, triangle interlock, concave mixed nesting, real outlines, rotations, spacing, margins and invalid inputs.\n";
        return 0;
    }
    catch (const std::exception& error) { std::cerr<<error.what()<<'\n'; return 1; }
}
