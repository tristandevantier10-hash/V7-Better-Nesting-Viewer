#include "../NestingGeometry.h"
#include "../NestingGeometryBridge.h"
#include "../NestingLayoutSearch.h"
#include "../Job.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

using namespace NestingGeometry;
void check(bool condition, const char* message)
{ if (!condition) throw std::runtime_error(message); }
template<class F> void rejects(F operation)
{
    bool rejected=false;
    try { operation(); } catch (const std::invalid_argument&) { rejected=true; }
    check(rejected,"Invalid geometry must be rejected explicitly.");
}

int main()
{
    try
    {
        const Polygon square=rectangle(10,10);
        check(area(square)==100,"Rectangle area.");
        auto clockwise=square;
        std::reverse(clockwise.vertices.begin(),clockwise.vertices.end());
        check(area(clockwise)==100 && collide(square,clockwise),"Winding must not affect collision.");
        auto closed=square; closed.vertices.push_back(closed.vertices.front());
        check(validateOutline(closed).empty(),"Explicitly closed rings must be accepted.");
        Polygon collinear{{{0,0},{5,0},{10,0},{10,10},{0,10}}};
        check(collide(collinear,square),"Redundant collinear vertices.");
        check(!validateOutline({{{0,0},{5,5},{0,5},{5,0}}}).empty(),"Reject self-crossing outline.");
        check(!validateOutline({{{0,0},{1,0},{2,0}}}).empty(),"Reject zero-area outline.");
        check(!validateOutline({{{0,0},{0,0},{1,1},{0,1}}}).empty(),"Reject zero-length edge.");
        check(!validateOutline({{{0,0},{10,0},{5,0},{5,5},{0,5}}}).empty(),"Reject backtracking edge.");
        rejects([] { rectangle(-1,2); });
        rejects([&] { transformed(square,{0,0,std::numeric_limits<double>::quiet_NaN()}); });
        rejects([&] { collide(square,{},0); });
        rejects([&] { collide(square,square,-1); });

        Polygon elbow{{{0,0},{10,0},{10,3},{3,3},{3,10},{0,10}}};
        const Polygon inNotch=transformed(rectangle(4,4),{4,4,0});
        check(area(elbow)==51,"Concave outline area uses actual geometry.");
        check(!collide(elbow,inNotch),"Overlapping bounding boxes are not a contour collision.");
        check(collide(elbow,transformed(rectangle(1,1),{1,5,0})),"Detect containment in concave arm.");
        check(collide(elbow,transformed(rectangle(12,1),{-1,1,0})),"Detect edge crossings.");
        check(collide(square,square),"Coincident contours must collide.");
        check(!collide(square,transformed(square,{10,0,0})),"Shared edges permitted at zero gap.");
        check(!collide(square,transformed(square,{10,10,0})),"Shared vertex permitted at zero gap.");
        const Polygon next=transformed(rectangle(2,2),{11,0,0});
        check(!collide(square,next,1),"Exact requested clearance is permitted.");
        check(collide(square,next,1.01),"Insufficient clearance must collide.");
        check(collide(square,transformed(square,{10,0,0}),0.1),"Positive gap prohibits contact.");
        for (int x=-8;x<=8;++x) for (int y=-8;y<=8;++y)
        {
            const double left=x*0.5, top=y*0.5;
            const bool expected=std::min(4.0,left+3)-std::max(0.0,left)>tolerance &&
                std::min(3.0,top+2)-std::max(0.0,top)>tolerance;
            check(collide(rectangle(4,3),transformed(rectangle(3,2),{left,top,0}))==expected,
                "Contour collision must agree with rectangle overlap across contact/containment cases.");
        }
        const Polygon turned=transformed(rectangle(2,4),{4,0,90});
        check(fitsSheet(turned,4,2),"Quarter-turn transform and boundary tolerance.");
        check(!fitsSheet(turned,4,2,0.1),"Margins apply to real geometry.");
        check(!fitsSheet(transformed(square,{-0.01,0,0}),10,10),"Reject negative placement.");
        check(fitsSheet(transformed(rectangle(2,2),{5,5,45}),10,10),"Arbitrary-angle transform.");
        check(methodFor({{1,square,1,{0}}})==Method::Rectangles,"Rectangle method dispatch.");
        check(methodFor({{1,square,1,{0}},{2,elbow,1,{0}}})==Method::Contours,"Mixed job needs contours.");
        rejects([&] { methodFor({{1,elbow,1,{0}},{2,{},1,{0}}}); });

        std::vector<Part> parts{{101,elbow,1,{0,90}},{202,rectangle(4,4),1,{0,90}}};
        std::vector<SheetLayout> layout{{10,10,{{101,0,{0,0,0}},{202,0,{4,4,0}}}}};
        check(validateLayout(parts,layout).empty(),"Validate nested concave arrangement by contours.");
        auto invalid=layout;
        invalid[0].placements[1].transform={1,5,0};
        check(!validateLayout(parts,invalid).empty(),"Reject contour overlap in complete layout.");
        invalid=layout; invalid[0].placements.push_back(invalid[0].placements.front());
        check(!validateLayout(parts,invalid).empty(),"Reject duplicate physical instance.");
        invalid=layout; invalid[0].placements.pop_back();
        check(!validateLayout(parts,invalid).empty(),"Reject missing instance.");
        invalid=layout; invalid[0].placements[1].partId=999;
        check(!validateLayout(parts,invalid).empty(),"Reject unknown source identity.");
        invalid=layout; invalid[0].placements[1].transform.rotationDegrees=45;
        check(!validateLayout(parts,invalid).empty(),"Reject disallowed rotation.");

        // Equal-size sources must remain distinguishable after every search pass.
        std::vector<Rect> demand{{500,500,4,1001},{500,500,4,1002},{300,100,28,1003}};
        auto packed=NestingLayoutSearch::findBest(demand,2440,1220);
        check(packed.isValid(),"Identity-aware rectangle search.");
        check(NestingLayoutSearch::containsAll(packed,demand),"Preserve equal-size source identities.");
        check(validateLayout(rectangleParts(demand),contourLayouts(packed.sheets)).empty(),
            "Rectangle result must pass independent contour and instance validation.");
        std::vector<Rect> fullJob{{500,500,50,2001},{300,100,34,2002}};
        packed=NestingLayoutSearch::findBest(fullJob,2440,1220);
        check(packed.isValid() && packed.sheets.front().placed.size()==37,"Preserve multi-sheet extra fit.");
        check(validateLayout(rectangleParts(fullJob),contourLayouts(packed.sheets)).empty(),
            "Transfers and cleanup must preserve all 84 source instances.");
        auto rotated=NestingLayoutSearch::findBest({{600,400,1,3001}},400,600);
        check(validateLayout(rectangleParts({{600,400,1,3001}}),contourLayouts(rotated.sheets)).empty(),
            "Rectangle-to-contour bridge must preserve rotated origin.");
        check(!NestingLayoutSearch::findBest({{100,100,1,7},{100,100,1,7}},1000,1000).isValid(),
            "Reject duplicate source definitions.");
        JobItem first{}, second{};
        auto copied=first;
        check(first.partId!=0 && first.partId!=second.partId && copied.partId==first.partId,
            "New sources get unique IDs; job snapshots preserve IDs.");
        std::cout << "Geometry checks passed: concavity, collision, contact, clearance, transforms, sheet bounds, routing, source/copy identities and rectangle compatibility.\n";
        return 0;
    }
    catch (const std::exception& error)
    { std::cerr << error.what() << '\n'; return 1; }
}
