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
        check(equivalent(square,collinear) && equivalent(square,closed) && equivalent(square,clockwise),
            "Equivalent boundaries ignore collinear samples, closure and winding.");
        auto normalisedRotation=[](const Polygon& polygon,double angle)
        {
            auto rotated=transformed(polygon,{0,0,angle});
            const auto box=bounds(rotated);
            return transformed(rotated,{-box.left,-box.top,0});
        };
        check(equivalent(square,normalisedRotation(square,90)),"Square quarter-turns are duplicate geometry.");
        const auto longRectangle=rectangle(20,10);
        check(equivalent(longRectangle,normalisedRotation(longRectangle,180)),"Rectangle half-turns are duplicate geometry.");
        check(!equivalent(longRectangle,normalisedRotation(longRectangle,90)),"Retain the distinct rectangle quarter-turn.");
        Polygon centered=rectangle(10,10);
        centered.holes={transformed(rectangle(4,4),{3,3,0})};
        check(equivalent(centered,normalisedRotation(centered,90)),"Symmetric DXF frames can share quarter-turn geometry.");
        auto asymmetric=centered;
        asymmetric.holes={transformed(rectangle(2,3),{1,2,0})};
        check(!equivalent(asymmetric,normalisedRotation(asymmetric,90)) &&
            !equivalent(asymmetric,normalisedRotation(asymmetric,180)),"Matching outer bounds must not hide a moved or rotated cutout.");
        auto slot=centered;
        slot.holes={transformed(rectangle(6,2),{2,4,0})};
        check(!equivalent(slot,normalisedRotation(slot,90)) && equivalent(slot,normalisedRotation(slot,180)),
            "Hole shape determines which rotations remain distinct.");
        Polygon twin=rectangle(10,10);
        twin.holes={transformed(rectangle(2,2),{1,4,0}),transformed(rectangle(2,2),{7,4,0})};
        auto reordered=twin;
        std::reverse(reordered.holes.begin(),reordered.holes.end());
        for (auto& hole:reordered.holes)
        {
            std::rotate(hole.vertices.begin(),hole.vertices.begin()+1,hole.vertices.end());
            std::reverse(hole.vertices.begin(),hole.vertices.end());
        }
        check(equivalent(twin,reordered) && equivalent(twin,normalisedRotation(twin,180)),
            "Hole order, ring starts and winding must not defeat symmetry detection.");
        check(!equivalent(twin,normalisedRotation(twin,90)),"Retain distinct multi-hole arrangements.");
        reordered.holes[0].vertices[0].x+=0.0001;
        check(!equivalent(twin,reordered),"Retain small but meaningful boundary differences.");
        Polygon hexagon;
        for (int i=0;i<6;++i) hexagon.vertices.push_back({10+10*std::cos(i*3.141592653589793/3),10+10*std::sin(i*3.141592653589793/3)});
        check(equivalent(normalisedRotation(hexagon,0),normalisedRotation(hexagon,60)),"Non-quarter-turn symmetries are eligible too.");
        check(!equivalent(normalisedRotation(circle(10),0),normalisedRotation(circle(10),1)),"Retain genuinely distinct faceted circle orientations.");
        rejects([&] { equivalent(square,Polygon{}); });
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
        const auto disk=circle(200);
        check(!collide(disk,transformed(disk,{206,0,0}),6),"Exact gap between faceted circles is allowed.");
        check(collide(disk,transformed(disk,{205.99,0,0}),6),"Circle gap below threshold collides.");
        check(!collide(disk,transformed(disk,{206.01,0,0}),6),"Separated circle disks are rejected quickly.");
        const Polygon largeTriangle{{{0,0},{600,0},{0,400}}};
        const auto aboveTriangle=transformed(disk,{200,-206,0});
        check(!collide(aboveTriangle,largeTriangle,6) && !collide(largeTriangle,aboveTriangle,6),"Exact circle-to-triangle gap in either order.");
        check(collide(transformed(disk,{200,-205.99,0}),largeTriangle,6),"Circle-to-triangle gap below threshold.");
        const auto nearCorner=transformed(disk,{-175,-175,0});
        check(!collide(nearCorner,largeTriangle,6),"Enclosing disk cannot falsely collide near a polygon corner.");
        check(collide(largeTriangle,nearCorner,6.1),"Near-corner clearance uses actual polygon edges.");
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
        Polygon frame=rectangle(10,10);
        frame.holes={transformed(rectangle(6,6),{2,2,0})};
        check(validateOutline(frame).empty() && area(frame)==64,"Subtract cutout area.");
        check(!isRectangle(frame) && methodFor({{3,frame,1,{0,90}}})==Method::Contours,"Frames never use rectangle packing.");
        const auto plug=transformed(rectangle(4,4),{3,3,0});
        check(!collide(frame,plug) && !collide(plug,frame),"Cutout interiors are empty in either argument order.");
        check(!collide(frame,plug,1) && collide(frame,plug,1.01),"Clearance to inner walls.");
        check(!collide(frame,frame.holes[0]) && collide(frame,frame.holes[0],0.01),"Exact cutout fit allows contact only at zero gap.");
        check(collide(frame,transformed(rectangle(2,2),{1,4,0})),"Crossing a hole wall collides.");
        check(collide(frame,frame),"Coincident frames still collide.");
        Polygon innerFrame=transformed(rectangle(5,5),{2.5,2.5,0});
        innerFrame.holes={transformed(rectangle(3,3),{3.5,3.5,0})};
        check(!collide(frame,innerFrame,0.5),"A perforated part can also nest inside a cutout.");
        auto reversedFrame=frame;
        std::reverse(reversedFrame.vertices.begin(),reversedFrame.vertices.end());
        std::reverse(reversedFrame.holes[0].vertices.begin(),reversedFrame.holes[0].vertices.end());
        check(area(reversedFrame)==64 && !collide(reversedFrame,plug),"Hole winding is immaterial.");
        for (double angle:{0.0,37.0,90.0,180.0,270.0})
        {
            const Transform pose{40,-15,angle};
            check(!collide(transformed(frame,pose),transformed(plug,pose),1),"Rotate and translate holes with their parent.");
            check(collide(transformed(frame,pose),transformed(plug,pose),1.01),"Rotated inner clearance.");
        }
        check(scanline(frame,5)==std::vector<double>({0,2,8,10}),"Preview fill excludes cutouts.");
        check(scanline(frame,1)==std::vector<double>({0,10}) && scanline(frame,10).empty(),"Half-open scanline boundary convention.");
        auto invalidHole=frame; invalidHole.holes[0]=transformed(rectangle(6,6),{0,2,0});
        check(!validateOutline(invalidHole).empty(),"Reject holes touching outer boundary.");
        invalidHole=frame; invalidHole.holes[0]=transformed(rectangle(6,6),{5,2,0});
        rejects([&] { collide(invalidHole,plug); });
        invalidHole=frame; invalidHole.holes.push_back(transformed(rectangle(2,2),{3,3,0}));
        check(!validateOutline(invalidHole).empty(),"Reject nested hole rings.");
        invalidHole=frame; invalidHole.holes.push_back(transformed(rectangle(2,2),{7,3,0}));
        check(!validateOutline(invalidHole).empty(),"Reject intersecting holes.");
        invalidHole=frame; invalidHole.holes[0].holes={rectangle(1,1)};
        check(!validateOutline(invalidHole).empty(),"Reject recursive hole topology.");
        Polygon slanted=rectangle(20,20);
        slanted.holes={{{{5,5},{15,5},{10,15}}}};
        check(!collide(slanted,transformed(rectangle(1,1),{9,8,0})),"Sloped hole edges.");
        check(collide(slanted,transformed(rectangle(3,3),{5,10,0})),"Material beside a sloped hole.");
        Polygon twoHoles=rectangle(20,10);
        twoHoles.holes={transformed(rectangle(6,6),{2,2,0}),transformed(rectangle(6,6),{12,2,0})};
        check(area(twoHoles)==128 && !collide(twoHoles,plug),"Multiple cutouts retain the material bridge.");
        check(collide(twoHoles,transformed(rectangle(2,2),{9,4,0})),"Bridge between holes is occupied.");
        auto scaledFrame=frame;
        mapVertices(scaledFrame,[](Point p) { return Point{p.x*3,p.y*2}; });
        check(area(scaledFrame)==384 && bounds(scaledFrame.holes[0]).right==24,"Resize outer and inner rings together.");
        // Independent material oracle: a frame is exactly four solid strips.
        const std::vector<Polygon> strips{rectangle(10,2),transformed(rectangle(10,2),{0,8,0}),
            transformed(rectangle(2,6),{0,2,0}),transformed(rectangle(2,6),{8,2,0})};
        const CollisionShape preparedFrame(frame),preparedProbe(rectangle(1,1));
        for (int x=-2;x<=20;++x) for (int y=-2;y<=20;++y)
        {
            auto probe=transformed(rectangle(1,1),{x*0.5,y*0.5,0});
            bool expected=false;
            for (const auto& strip:strips) expected=expected || collide(strip,probe);
            check(collide(frame,probe)==expected && collide(probe,frame)==expected,"Hole collision agrees with independent solid-strip oracle.");
            const Point origin{12,-7},position{12+x*0.5,-7+y*0.5};
            check(collide(preparedFrame,origin,preparedProbe,position)==expected &&
                collide(preparedProbe,position,preparedFrame,origin)==expected,"Prepared collision preserves translated material and argument symmetry.");
        }
        const CollisionShape preparedPlug(rectangle(4,4));
        check(!collide(preparedFrame,{},preparedPlug,{3,3},1) && collide(preparedFrame,{},preparedPlug,{3,3},1.01),
            "Prepared geometry preserves inner clearance thresholds.");
        rejects([&] { collide(preparedFrame,{},preparedPlug,{std::numeric_limits<double>::quiet_NaN(),0}); });
        rejects([&] { CollisionShape invalid(Polygon{}); });
        auto perforatedElbow=elbow;
        perforatedElbow.holes={transformed(rectangle(1,1),{1,1,0})};
        check(validateOutline(perforatedElbow).empty() && !collide(perforatedElbow,inNotch),"Concave outer outline with a hole.");
        check(!collide(perforatedElbow,perforatedElbow.holes[0]),"Concave outer cutout remains empty.");
        invalidHole=elbow; invalidHole.holes={inNotch};
        check(!validateOutline(invalidHole).empty(),"A hole inside outer bounds but outside concave material is invalid.");
        invalidHole=frame; invalidHole.holes[0].vertices[0].x=std::numeric_limits<double>::quiet_NaN();
        check(!validateOutline(invalidHole).empty(),"Nonfinite hole geometry is rejected.");
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
