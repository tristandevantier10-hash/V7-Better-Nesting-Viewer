#include "../NestingPairSearch.h"
#include "../NestingService.h"
#include "../NestingGeometryBridge.h"
#include "../DxfImport.h"
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace NestingGeometry;
void check(bool value,const char* message) { if (!value) throw std::runtime_error(message); }
std::vector<NestingPairSearch::Pair> proposals(const std::vector<Part>& parts,double gap=0)
{
    NestingPairSearch::Variants variants;
    for (const auto& part:parts) for (double angle:part.allowedRotations)
    {
        const auto rotated=transformed(part.outline,{0,0,angle});
        const auto rb=bounds(rotated);
        auto local=transformed(rotated,{-rb.left,-rb.top,0});
        CollisionShape collision(local);
        variants[part.id].push_back({std::move(local),rb,std::move(collision),angle,false});
    }
    auto pairs=NestingPairSearch::find(parts,variants,1000,1000,gap);
    for (const auto& pair:pairs)
    {
        std::vector<Part> definitions{parts[pair.first]};
        definitions[0].quantity=pair.first==pair.second ? 2 : 1;
        if (pair.first!=pair.second) { definitions.push_back(parts[pair.second]); definitions.back().quantity=1; }
        SheetLayout layout{pair.width,pair.height,{{parts[pair.first].id,0,pair.a},
            {parts[pair.second].id,pair.first==pair.second ? 1u : 0u,pair.b}}};
        check(validateLayout(definitions,{layout},gap).empty(),"Pair proposal violates real geometry, identity or rotations.");
    }
    return pairs;
}
void verify(const std::vector<Part>& parts,const NestingService::Options& options,size_t sheets=1)
{
    const auto result=NestingService::calculate(parts,options);
    check(result.layout.isValid() && result.layout.sheets.size()==sheets,"Expected complete grouped layout on the requested sheet count.");
    check(validateLayout(parts,contourLayouts(result.layout.sheets),options.clearance,options.margin).empty(),
        "Expanded groups must preserve quantities, identities, rotations, margins and real clearance.");
}
std::string poly(const Polygon& polygon)
{
    std::ostringstream out; out<<"0\nLWPOLYLINE\n90\n"<<polygon.vertices.size()<<"\n70\n1\n";
    for (const auto& p:polygon.vertices) out<<"10\n"<<p.x<<"\n20\n"<<p.y<<"\n";
    return out.str();
}
std::string document(const std::string& entities)
{ return "0\nSECTION\n2\nHEADER\n9\n$INSUNITS\n70\n4\n0\nENDSEC\n0\nSECTION\n2\nENTITIES\n"+entities+"0\nENDSEC\n0\nEOF\n"; }
int main() try
{
    Polygon elbow{{{0,0},{100,0},{100,20},{20,20},{20,100},{0,100}}};
    std::vector<Part> mixed{{101,elbow,6,{0},"Unrecognised DXF name"},{102,rectangle(80,80),6,{0},"Insert"}};
    const auto pairs=proposals(mixed);
    check(std::any_of(pairs.begin(),pairs.end(),[](const auto& p) { return p.first!=p.second && p.width*p.height<=10000+tolerance; }),
        "Generic pairing must discover a different part fitting the L-shaped recess.");
    verify(mixed,{300,200}); // The ungrouped search needs two sheets for this fixture.
    auto multiSheet=mixed;
    multiSheet[0].quantity=12; multiSheet[1].quantity=12;
    verify(multiSheet,{300,200},2);
    Polygon wideElbow{{{0,0},{100,0},{100,20},{20,20},{20,60},{0,60}}};
    check(!NestingService::calculate({{111,wideElbow,1,{0},"Fixed L"},{112,rectangle(80,40),1,{0},"Fixed insert"}},
        {60,100}).layout.isValid(),"A group cannot rotate members beyond their allowed orientations.");
    mixed[0].outline=transformed(elbow,{-17,23,0});
    mixed[1].outline=rectangle(76,76);
    verify(mixed,{308,206,2,2});
    mixed[1].quantity=5;
    std::reverse(mixed.begin(),mixed.end());
    verify(mixed,{308,206,2,2}); // Unpaired remainder and reversed source order.

    Polygon irregular{{{0,0},{100,0},{70,40},{40,100},{0,100}}};
    Polygon complement{{{100,0},{100,100},{40,100},{70,40}}};
    auto imported=DxfImport::parse(document(poly(irregular)+poly(transformed(complement,{200,0,0}))));
    check(imported.valid() && imported.outlines.size()==2,"Import arbitrary non-triangular profiles.");
    std::vector<Part> dxfParts{{201,imported.outlines[0].polygon,6,{0},"Imported pentagon"},
        {202,imported.outlines[1].polygon,6,{0},"Imported quadrilateral"}};
    auto irregularPairs=proposals(dxfParts);
    check(std::any_of(irregularPairs.begin(),irregularPairs.end(),[](const auto& p) { return p.first!=p.second; }),
        "Imported arbitrary polygons must participate in pair search.");
    verify(dxfParts,{300,200});

    auto concaveDxf=DxfImport::parse(document(poly(elbow)+poly(transformed(rectangle(76,76),{200,0,0}))));
    check(concaveDxf.valid(),"Import concave pair fixture.");
    verify({{301,concaveDxf.outlines[0].polygon,6,{0},"DXF A"},{302,concaveDxf.outlines[1].polygon,6,{0},"DXF B"}},
        {308,206,2,2});

    auto holes=DxfImport::parse(document(poly(rectangle(100,100))+poly(transformed(rectangle(60,60),{20,20,0}))+ 
        poly(transformed(rectangle(56,56),{200,0,0}))));
    check(holes.valid() && holes.outlines[0].polygon.holes.size()==1,"Import hole-bearing pair fixture.");
    std::vector<Part> perforated{{401,holes.outlines[0].polygon,3,{0,90},"DXF frame"},
        {402,holes.outlines[1].polygon,3,{0,90},"DXF insert"}};
    auto holePairs=proposals(perforated,2);
    check(std::any_of(holePairs.begin(),holePairs.end(),[](const auto& p) { return p.first!=p.second; }),
        "Hole boundaries must produce validated cross-source groups.");
    verify(perforated,{310,106,2,3});
    perforated[0].quantity=1; perforated[1].quantity=1; perforated[1].outline=rectangle(61,61);
    check(proposals(perforated,2).empty(),"A group must not force an oversized insert into a cutout.");

    // Pair rotations come from each source, including non-quarter turns.
    auto angled=transformed(elbow,{13,-7,30});
    std::vector<Part> nonOrthogonal{{501,angled,2,{30,210},"Angled concave profile"}};
    check(!proposals(nonOrthogonal,1).empty(),"Arbitrary allowed rotations remain eligible for grouping.");
    verify(nonOrthogonal,{400,400,1,5});
    std::cout<<"General grouping passed: concavity, arbitrary DXFs, holes, mixed sources, fixed/arbitrary rotations, remainders and identities.\n";
    return 0;
}
catch (const std::exception& error) { std::cerr<<error.what()<<'\n'; return 1; }
