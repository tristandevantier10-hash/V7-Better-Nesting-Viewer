#include "NestingService.h"
#include "NestingLayoutSearch.h"
#include "NestingPairSearch.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <memory>
#include <set>
#include <tuple>
#include <exception>

namespace NestingService
{
    namespace
    {
        using namespace NestingGeometry;
        bool allows(const Part& part, double requested)
        {
            for (double angle : part.allowedRotations)
                if (std::abs(std::remainder(angle-requested,360.0)) <= tolerance) return true;
            return false;
        }
        bool boxesSeparated(const Bounds& a, const Bounds& b, double gap)
        {
            return a.right+gap < b.left-tolerance || b.right+gap < a.left-tolerance ||
                a.bottom+gap < b.top-tolerance || b.bottom+gap < a.top-tolerance;
        }
        struct Occupied { Polygon polygon; Bounds box; CollisionShape collision; bool round; };
        bool triangleLike(const Polygon& polygon)
        {
            if (!polygon.holes.empty()) return false;
            if (polygon.vertices.size()==3) return true;
            if (polygon.vertices.size()!=4) return false;
            const auto box=bounds(polygon);
            const double shortEdge=0.001*std::max(box.right-box.left,box.bottom-box.top);
            for (size_t i=0;i<polygon.vertices.size();++i)
            {
                const auto a=polygon.vertices[i],b=polygon.vertices[(i+1)%polygon.vertices.size()];
                if (std::hypot(b.x-a.x,b.y-a.y)<shortEdge) return true;
            }
            return false;
        }
        bool circleEnvelope(const Polygon& polygon,const Bounds& box)
        {
            if (!polygon.holes.empty() || polygon.vertices.size()!=64) return false;
            const double diameter=box.right-box.left;
            if (std::abs(diameter-(box.bottom-box.top))>tolerance) return false;
            const double cx=(box.left+box.right)/2,cy=(box.top+box.bottom)/2;
            const double radius=diameter/2/std::cos(3.14159265358979323846/64);
            for (const auto& p:polygon.vertices)
                if (std::abs(std::hypot(p.x-cx,p.y-cy)-radius)>tolerance) return false;
            return true;
        }
        struct Trial
        {
            std::vector<SheetLayout> sheets;
            std::vector<std::vector<Occupied>> occupied;
            bool complete = true;
        };
        using Variant=NestingPairSearch::Variant;
        using Variants=NestingPairSearch::Variants;
        Variants prepareVariants(const std::vector<Part>& parts)
        {
            Variants result;
            for (const auto& part:parts)
            {
                auto& variants=result[part.id];
                for (double angle:part.allowedRotations)
                {
                    const auto rotated=transformed(part.outline,{0,0,angle});
                    const auto rb=bounds(rotated);
                    auto local=transformed(rotated,{-rb.left,-rb.top,0});
                    const bool round=circleEnvelope(rotated,rb);
                    bool duplicate=false;
                    for (const auto& tested:variants)
                    {
                        if (equivalent(local,tested.local)) { duplicate=true; break; }
                    }
                    if (duplicate) continue;
                    CollisionShape collision(local);
                    variants.push_back({std::move(local),rb,std::move(collision),angle,round});
                }
            }
            return result;
        }
        bool place(const Part& part, std::uint32_t instance, SheetLayout& sheet,
            std::vector<Occupied>& occupied, const Options& options, bool bottomFirst, bool gridOnly,
            const std::vector<Variant>& variants, bool leftAligned=false)
        {
            bool found = false;
            Transform bestTransform;
            Polygon bestPolygon;
            Bounds bestBounds;
            std::tuple<double,double,double,double> bestScore;
            double previousRight=options.margin, previousBottom=options.margin;
            for (const auto& p:occupied)
            { previousRight=std::max(previousRight,p.box.right); previousBottom=std::max(previousBottom,p.box.bottom); }
            for (const auto& variant:variants)
            {
                const auto& rb=variant.rotatedBounds;
                const auto& local=variant.local;
                const auto& localCollision=variant.collision;
                const auto angle=variant.angle;
                const double width=rb.right-rb.left, height=rb.bottom-rb.top;
                const bool round=variant.round;
                if (width > sheet.width-2*options.margin+tolerance || height > sheet.height-2*options.margin+tolerance) continue;
                std::set<std::pair<double,double>> anchors;
                auto add = [&](double x,double y)
                {
                    if (leftAligned && std::abs(x-options.margin)>tolerance) return;
                    if (anchors.size()>=2048 || x<options.margin-tolerance || y<options.margin-tolerance ||
                        x+width>sheet.width-options.margin+tolerance || y+height>sheet.height-options.margin+tolerance) return;
                    // Collapse roundoff from quarter-turn rotations only.
                    if (std::abs(x-options.margin)<tolerance) x=options.margin;
                    if (std::abs(y-options.margin)<tolerance) y=options.margin;
                    anchors.emplace(x,y);
                };
                add(options.margin,options.margin);
                add(sheet.width-options.margin-width,options.margin);
                add(options.margin,sheet.height-options.margin-height);
                // Reserve cutout candidates before the bounded contact/grid
                // search fills up. Centre anchors also handle round cutouts.
                for (const auto& previous:occupied)
                {
                    for (const auto& hole:previous.polygon.holes)
                    {
                        const auto hb=bounds(hole);
                        for (double x:{hb.left+options.clearance,hb.right-width-options.clearance,(hb.left+hb.right-width)/2})
                            for (double y:{hb.top+options.clearance,hb.bottom-height-options.clearance,(hb.top+hb.bottom-height)/2}) add(x,y);
                    }
                    // Also try placing a frame around a part already laid out.
                    for (const auto& hole:local.holes)
                    {
                        const auto hb=bounds(hole);
                        for (double x:{previous.box.left-hb.left-options.clearance,previous.box.right-hb.right+options.clearance,
                            (previous.box.left+previous.box.right-hb.left-hb.right)/2})
                            for (double y:{previous.box.top-hb.top-options.clearance,previous.box.bottom-hb.bottom+options.clearance,
                                (previous.box.top+previous.box.bottom-hb.top-hb.bottom)/2}) add(x,y);
                    }
                }
                std::set<double> xs{options.margin}, ys{options.margin};
                for (const auto& previous:occupied)
                {
                    for (double x:{previous.box.left,previous.box.right+options.clearance,
                        previous.box.left-width-options.clearance,previous.box.right-width}) xs.insert(x);
                    for (double y:{previous.box.top,previous.box.bottom+options.clearance,
                        previous.box.top-height-options.clearance,previous.box.bottom-height}) ys.insert(y);
                }
                // Reserve simple edge/grid placements before detailed contacts
                // fill the bounded candidate set (notably for circle outlines).
                for (double y:ys) for (double x:xs) add(x,y);
                if (!gridOnly && triangleLike(local))
                    for (const auto& previous:occupied)
                    {
                        if (!triangleLike(previous.polygon)) continue;
                        // Keep one axis aligned while sliding a vertex onto an edge.
                        // Imported triangles can have nearly matching endpoints.
                        for (bool transpose:{false,true})
                        {
                            const auto axis=[transpose](Point p) { return transpose ? p.y : p.x; };
                            const auto slide=[transpose](Point p) { return transpose ? p.x : p.y; };
                            const double low=transpose ? previous.box.top : previous.box.left;
                            const double high=transpose ? previous.box.bottom : previous.box.right;
                            const double span=transpose ? height : width;
                            const double limit=transpose ? sheet.height : sheet.width;
                            for (double fixed:{options.margin,limit-options.margin-span,low,high-span})
                                for (bool movingEdge:{false,true})
                                {
                                    const auto& edges=movingEdge ? local.vertices : previous.polygon.vertices;
                                    const auto& vertices=movingEdge ? previous.polygon.vertices : local.vertices;
                                    const double direction=movingEdge ? -1.0 : 1.0;
                                    for (size_t i=0;i<edges.size();++i)
                                    {
                                        const Point a=edges[i],b=edges[(i+1)%edges.size()];
                                        const double run=axis(b)-axis(a);
                                        if (std::abs(run)<=tolerance) continue;
                                        const double slope=(slide(b)-slide(a))/run;
                                        const double gap=options.clearance*std::hypot(1.0,slope);
                                        for (Point vertex:vertices)
                                        {
                                            const double along=axis(vertex)+direction*fixed-axis(a);
                                            const double contact=slide(a)+slope*along;
                                            const double offset=direction*(contact-slide(vertex));
                                            for (double position:{offset-gap,offset+gap})
                                            {
                                                const double projection=(along+direction*(position-offset)*slope/(1+slope*slope))/run;
                                                if (projection < -tolerance || projection > 1+tolerance) continue;
                                                if (transpose) add(position,fixed);
                                                else add(fixed,position);
                                            }
                                        }
                                    }
                                }
                        }
                    }
                for (const auto& previous:occupied)
                {
                    if (round && previous.round)
                    {
                        // Circle contacts need a small angular search, rather
                        // than thousands of near-duplicate polygon vertex pairs.
                        if (!gridOnly)
                        {
                            const double distance=(width+previous.box.right-previous.box.left)/2/
                                std::cos(3.14159265358979323846/64)+options.clearance;
                            for (int step=0;step<24;++step)
                            {
                                const double angle=step*3.14159265358979323846/12;
                                add((previous.box.left+previous.box.right-width)/2+distance*std::cos(angle),
                                    (previous.box.top+previous.box.bottom-height)/2+distance*std::sin(angle));
                            }
                        }
                        continue;
                    }
                    // Vertex contacts can place a part inside a concave notch,
                    // even when its bounding box overlaps another bounding box.
                    if (!gridOnly) for (const auto* previousRing:boundaryRings(previous.polygon))
                      for (const auto* localRing:boundaryRings(local))
                       for (const auto& p:previousRing->vertices)
                        for (const auto& q:localRing->vertices)
                        {
                            add(p.x-q.x,p.y-q.y);
                            if (options.clearance>0)
                            {
                                for (double dx:{-options.clearance,0.0,options.clearance})
                                    for (double dy:{-options.clearance,0.0,options.clearance}) add(p.x-q.x+dx,p.y-q.y+dy);
                            }
                        }
                }
                for (double y:ys) for (double x:xs) add(x,y);
                struct Candidate { double x,y; std::tuple<double,double,double,double> score; };
                std::vector<Candidate> candidates;
                for (const auto& anchor:anchors)
                {
                    const double right=std::max(previousRight,anchor.first+width);
                    const double bottom=std::max(previousBottom,anchor.second+height);
                    candidates.push_back({anchor.first,anchor.second,bottomFirst
                        ? std::make_tuple(bottom,right,anchor.second,anchor.first)
                        : std::make_tuple(right*bottom,bottom,anchor.second,anchor.first)});
                }
                std::stable_sort(candidates.begin(),candidates.end(),[](const Candidate& a,const Candidate& b)
                { return a.score<b.score; });
                for (const auto& candidate:candidates)
                {
                    if (found && !(candidate.score<bestScore)) break;
                    const Bounds box{candidate.x,candidate.y,candidate.x+width,candidate.y+height};
                    bool collision=false;
                    for (const auto& previous:occupied)
                        if (!boxesSeparated(box,previous.box,options.clearance) &&
                            collide(localCollision,{candidate.x,candidate.y},previous.collision,{},options.clearance))
                        { collision=true; break; }
                    if (collision) continue;
                    found=true;
                    bestScore=candidate.score;
                    bestTransform={candidate.x-rb.left,candidate.y-rb.top,angle};
                    bestBounds=box;
                    bestPolygon=transformed(local,{candidate.x,candidate.y,0});
                    break;
                }
            }
            if (!found) return false;
            sheet.placements.push_back({part.id,instance,bestTransform});
            auto collision=CollisionShape(bestPolygon);
            const bool round=circleEnvelope(bestPolygon,bestBounds);
            occupied.push_back({std::move(bestPolygon),bestBounds,std::move(collision),round});
            return true;
        }
        Trial contourTrial(const std::vector<Part>& parts, const Options& options, const Variants& variants,
            int order, bool bottomFirst, bool gridOnly=false)
        {
            std::vector<size_t> indices;
            for (size_t i=0;i<parts.size();++i) if (parts[i].quantity>0) indices.push_back(i);
            std::stable_sort(indices.begin(),indices.end(),[&](size_t a,size_t b)
            {
                if (order==1)
                {
                    const auto ab=bounds(parts[a].outline), bb=bounds(parts[b].outline);
                    return std::max(ab.right-ab.left,ab.bottom-ab.top)>std::max(bb.right-bb.left,bb.bottom-bb.top);
                }
                return order==2 ? area(parts[a].outline)<area(parts[b].outline) : area(parts[a].outline)>area(parts[b].outline);
            });
            Trial result;
            for (size_t index:indices)
                for (int copy=0;copy<parts[index].quantity;++copy)
                {
                    bool placed=false;
                    for (size_t sheet=0;sheet<result.sheets.size();++sheet)
                        if (place(parts[index],static_cast<std::uint32_t>(copy),result.sheets[sheet],result.occupied[sheet],options,bottomFirst,gridOnly,variants.at(parts[index].id)))
                        { placed=true; break; }
                    if (!placed)
                    {
                        result.sheets.push_back({options.sheetWidth,options.sheetHeight,{}});
                        result.occupied.emplace_back();
                        if (!place(parts[index],static_cast<std::uint32_t>(copy),result.sheets.back(),result.occupied.back(),options,bottomFirst,gridOnly,variants.at(parts[index].id)))
                        { result.complete=false; return result; }
                    }
                }
            return result;
        }
        struct Band
        {
            double width=0,height=0;
            std::vector<Placement> placements;
            bool staggered=false;
        };
        std::vector<Band> bandsFor(const Part& part,const std::vector<Variant>& variants,const Options& options,
            bool partialCircleBands=false)
        {
            std::vector<Band> bands;
            const double usableWidth=options.sheetWidth-2*options.margin;
            const double usableHeight=options.sheetHeight-2*options.margin;
            auto finish=[&](Band band)
            {
                if (band.width<=usableWidth+tolerance && band.height<=usableHeight+tolerance)
                    bands.push_back(std::move(band));
            };
            auto append=[&](Band& band,const Variant& variant,int instance,double x,double y)
            {
                const auto& box=variant.rotatedBounds;
                band.placements.push_back({part.id,static_cast<std::uint32_t>(instance),
                    {x-box.left,y-box.top,variant.angle}});
                band.width=std::max(band.width,x+box.right-box.left);
                band.height=std::max(band.height,y+box.bottom-box.top);
            };
            for (const auto& a:variants)
            {
                const double width=a.rotatedBounds.right-a.rotatedBounds.left;
                const double height=a.rotatedBounds.bottom-a.rotatedBounds.top;
                if (a.round)
                {
                    // Compare square rows and conservative hexagonal rows at
                    // every column count, rather than greedily growing a cluster.
                    for (bool stagger:{false,true}) for (bool transpose:{false,true})
                        for (int columns=1;columns<=part.quantity;++columns)
                        {
                            const double pitch=(stagger ? width/std::cos(3.14159265358979323846/64) : width)+options.clearance;
                            const double rowPitch=stagger ? pitch*std::sqrt(3.0)/2 : pitch;
                            if ((columns-1)*pitch+width>(transpose ? usableHeight : usableWidth)+tolerance &&
                                partialCircleBands) break;
                            Band band;
                            band.staggered=stagger;
                            for (int i=0;static_cast<int>(band.placements.size())<part.quantity;++i)
                            {
                                const int row=i/columns,column=i%columns;
                                double x=column*pitch+(stagger && row%2 ? pitch/2 : 0),y=row*rowPitch;
                                if (transpose) std::swap(x,y);
                                if (partialCircleBands && (x+width>usableWidth+tolerance || y+height>usableHeight+tolerance))
                                {
                                    if ((transpose ? x+width>usableWidth+tolerance : y+height>usableHeight+tolerance)) break;
                                    continue;
                                }
                                append(band,a,static_cast<int>(band.placements.size()),x,y);
                            }
                            finish(std::move(band));
                        }
                    continue;
                }
                if (!part.outline.holes.empty() || a.local.vertices.size()!=3) continue;
                // A right triangle occupies three corners of its bounding box.
                // Its allowed half-turn partner fills the complementary half.
                bool rightTriangle=true;
                for (const auto& p:a.local.vertices)
                    if ((std::abs(p.x)>tolerance && std::abs(p.x-width)>tolerance) ||
                        (std::abs(p.y)>tolerance && std::abs(p.y-height)>tolerance)) rightTriangle=false;
                if (!rightTriangle) continue;
                for (const auto& b:variants)
                {
                    if (std::abs(std::abs(std::remainder(a.angle-b.angle,360.0))-180)>tolerance ||
                        std::abs(b.rotatedBounds.right-b.rotatedBounds.left-width)>tolerance ||
                        std::abs(b.rotatedBounds.bottom-b.rotatedBounds.top-height)>tolerance) continue;
                    Point normal;
                    for (size_t edge=0;edge<3;++edge)
                    {
                        const auto p=a.local.vertices[edge],q=a.local.vertices[(edge+1)%3],third=a.local.vertices[(edge+2)%3];
                        if (std::abs(p.x-q.x)<width-tolerance || std::abs(p.y-q.y)<height-tolerance) continue;
                        const double length=std::hypot(q.x-p.x,q.y-p.y);
                        normal={-(q.y-p.y)/length,(q.x-p.x)/length};
                        if (normal.x*(third.x-p.x)+normal.y*(third.y-p.y)>0) { normal.x=-normal.x; normal.y=-normal.y; }
                    }
                    const double separation=options.clearance>0 ? options.clearance+10*tolerance : 0;
                    const Point shift{normal.x*separation,normal.y*separation};
                    if (collide(a.collision,{},b.collision,shift,options.clearance)) continue;
                    const double ax=std::max(0.0,-shift.x),ay=std::max(0.0,-shift.y);
                    const double tileWidth=width+std::abs(shift.x),tileHeight=height+std::abs(shift.y);
                    const int count=(part.quantity+1)/2;
                    for (int columns=1;columns<=count;++columns)
                    {
                        Band band;
                        for (int tile=0;tile<count;++tile)
                        {
                            const double x=(tile%columns)*(tileWidth+options.clearance),y=(tile/columns)*(tileHeight+options.clearance);
                            append(band,a,tile*2,x+ax,y+ay);
                            if (tile*2+1<part.quantity) append(band,b,tile*2+1,x+ax+shift.x,y+ay+shift.y);
                        }
                        finish(std::move(band));
                    }
                }
            }
            return bands;
        }
        Trial structuredTrial(const std::vector<Part>& parts,const Options& options,const Variants& variants,bool vertical)
        {
            Trial result;
            result.complete=false;
            SheetLayout sheet{options.sheetWidth,options.sheetHeight,{}};
            double offset=options.margin;
            for (const auto& part:parts)
            {
                if (part.quantity==0) continue;
                auto bands=bandsFor(part,variants.at(part.id),options);
                if (bands.empty()) return result;
                const auto best=std::min_element(bands.begin(),bands.end(),[&](const Band& a,const Band& b)
                {
                    return vertical ? std::make_pair(a.width,a.height)<std::make_pair(b.width,b.height)
                        : std::make_pair(a.height,a.width)<std::make_pair(b.height,b.width);
                });
                if (offset+(vertical ? best->width : best->height)>
                    (vertical ? options.sheetWidth : options.sheetHeight)-options.margin+tolerance) return result;
                for (auto placement:best->placements)
                {
                    placement.transform.x+=vertical ? offset : options.margin;
                    placement.transform.y+=vertical ? options.margin : offset;
                    sheet.placements.push_back(placement);
                }
                offset+=(vertical ? best->width : best->height)+options.clearance;
            }
            // Independent validation guards both grouping and all real gaps.
            if (!validateLayout(parts,{sheet},options.clearance,options.margin).empty()) return result;
            std::map<PartId,const Part*> definitions;
            for (const auto& part:parts) definitions[part.id]=&part;
            result.sheets.push_back(std::move(sheet));
            result.occupied.emplace_back();
            for (const auto& placement:result.sheets.front().placements)
            {
                auto polygon=transformed(definitions.at(placement.partId)->outline,placement.transform);
                const auto box=bounds(polygon);
                const bool round=circleEnvelope(polygon,box);
                CollisionShape collision(polygon);
                result.occupied.front().push_back({std::move(polygon),box,std::move(collision),round});
            }
            result.complete=true;
            return result;
        }
        struct GroupMember { size_t source; std::uint32_t instance; Transform local; };
        struct Group { Part packingPart; std::vector<GroupMember> members; };
        using GroupPattern=std::vector<std::pair<size_t,size_t>>;
        Trial groupedTrial(const std::vector<Part>& parts,const Options& options,const Variants& variants,
            std::vector<NestingPairSearch::Pair> pairs,int pairingOrder,bool bottomFirst,size_t maxSheets,
            std::set<GroupPattern>& visited)
        {
            Trial result;
            result.complete=false;
            if (pairs.empty()) return result;
            std::stable_sort(pairs.begin(),pairs.end(),[&](const auto& a,const auto& b)
            {
                if (pairingOrder==2 && (a.first==a.second)!=(b.first==b.second)) return a.first!=a.second;
                auto benefit=[&](const auto& pair)
                { return pairingOrder!=0 ? pair.saving/(pair.width*pair.height+pair.saving) : pair.saving; };
                return benefit(a)>benefit(b);
            });
            std::vector<int> remaining;
            for (const auto& part:parts) remaining.push_back(part.quantity);
            std::vector<Group> groups;
            for (const auto& pair:pairs)
                while (remaining[pair.first]>0 && remaining[pair.second]>(pair.first==pair.second ? 1 : 0))
                {
                    Group group;
                    group.packingPart={static_cast<PartId>(groups.size()+1),rectangle(pair.width,pair.height),1,{},"Geometry pair"};
                    for (double rotation:{0.0,90.0,180.0,270.0})
                        if (allows(parts[pair.first],pair.a.rotationDegrees+rotation) &&
                            allows(parts[pair.second],pair.b.rotationDegrees+rotation))
                            group.packingPart.allowedRotations.push_back(rotation);
                    const auto first=static_cast<std::uint32_t>(parts[pair.first].quantity-remaining[pair.first]--);
                    const auto second=static_cast<std::uint32_t>(parts[pair.second].quantity-remaining[pair.second]--);
                    group.members={{pair.first,first,pair.a},{pair.second,second,pair.b}};
                    groups.push_back(std::move(group));
                }
            for (size_t source=0;source<parts.size();++source)
                while (remaining[source]>0)
                {
                    const auto instance=static_cast<std::uint32_t>(parts[source].quantity-remaining[source]--);
                    auto packingPart=parts[source];
                    packingPart.id=static_cast<PartId>(groups.size()+1);
                    packingPart.quantity=1;
                    groups.push_back({std::move(packingPart),{{source,instance,{}}}});
                }
            GroupPattern pattern;
            double reservedArea=0;
            for (const auto& group:groups)
            {
                pattern.emplace_back(group.members.front().source,group.members.size()>1 ? group.members.back().source : parts.size());
                reservedArea+=area(group.packingPart.outline);
            }
            if (!visited.insert(std::move(pattern)).second || reservedArea>
                maxSheets*(options.sheetWidth-2*options.margin)*(options.sheetHeight-2*options.margin)+tolerance) return result;
            std::vector<Part> packingParts;
            for (const auto& group:groups) if (group.members.size()>1) packingParts.push_back(group.packingPart);
            auto packingVariants=prepareVariants(packingParts);
            for (const auto& group:groups)
                if (group.members.size()==1)
                    packingVariants.emplace(group.packingPart.id,variants.at(parts[group.members.front().source].id));
            std::vector<size_t> order;
            for (size_t index=0;index<groups.size();++index) order.push_back(index);
            std::stable_sort(order.begin(),order.end(),[&](size_t a,size_t b)
            { return area(groups[a].packingPart.outline)>area(groups[b].packingPart.outline); });
            for (size_t index:order)
            {
                const auto& group=groups[index].packingPart;
                bool placed=false;
                for (size_t sheet=0;sheet<result.sheets.size();++sheet)
                    if (place(group,0,result.sheets[sheet],result.occupied[sheet],options,bottomFirst,true,packingVariants.at(group.id)))
                    { placed=true; break; }
                if (!placed)
                {
                    // A grouped candidate needing more sheets than the best
                    // complete result cannot win the shared layout score.
                    if (result.sheets.size()>=maxSheets) return result;
                    result.sheets.push_back({options.sheetWidth,options.sheetHeight,{}});
                    result.occupied.emplace_back();
                    if (!place(group,0,result.sheets.back(),result.occupied.back(),options,bottomFirst,true,packingVariants.at(group.id))) return result;
                }
            }
            for (auto& sheet:result.sheets)
            {
                std::vector<Placement> expanded;
                for (const auto& placement:sheet.placements)
                {
                    const auto& group=groups.at(static_cast<size_t>(placement.partId-1));
                    const double angle=placement.transform.rotationDegrees*3.14159265358979323846/180;
                    const double cosine=std::cos(angle),sine=std::sin(angle);
                    for (const auto& member:group.members)
                        expanded.push_back({parts[member.source].id,member.instance,
                            {placement.transform.x+member.local.x*cosine-member.local.y*sine,
                             placement.transform.y+member.local.x*sine+member.local.y*cosine,
                             placement.transform.rotationDegrees+member.local.rotationDegrees}});
                }
                sheet.placements=std::move(expanded);
            }
            // Synthetic bounding boxes are only a packing aid. Restore actual
            // identities and geometry, then independently validate the result.
            result.complete=validateLayout(parts,result.sheets,options.clearance,options.margin).empty();
            return result;
        }
        auto trialScore(const Trial& trial)
        {
            double envelope=0, last=0;
            for (const auto& sheet:trial.occupied)
            {
                double right=0,bottom=0;
                for (const auto& part:sheet) { right=std::max(right,part.box.right); bottom=std::max(bottom,part.box.bottom); }
                last=right*bottom;
                envelope+=last;
            }
            // Bounding envelopes measure compactness, not consumed material:
            // a part inside a cutout must not add its box area a second time.
            return std::make_tuple(trial.sheets.size(),last,envelope);
        }
    }

    Result findCapacity(NestingGeometry::Part part, const Options& options)
    {
        part.quantity=1;
        auto best=calculate({part},options);
        if (!best.layout.isValid() || best.layout.sheets.size()!=1) return best;

        const int limit=best.method==NestingGeometry::Method::Rectangles ? 2000 : 200;
        const double usableArea=(options.sheetWidth-2*options.margin)*(options.sheetHeight-2*options.margin);
        const double areaBound=usableArea/NestingGeometry::area(part.outline);
        int low=1;
        int high=static_cast<int>(std::clamp(std::ceil(areaBound),1.0,static_cast<double>(limit)));
        if (best.method==NestingGeometry::Method::Contours)
        {
            using namespace NestingGeometry;
            const auto variants=prepareVariants({part});
            const auto& rotations=variants.at(part.id);
            Trial winner;
            bool winnerSimple=false;
            auto fill=[&](const std::vector<Variant>& allowed,bool bottomFirst,bool leftAligned,bool gridOnly)
            {
                Trial trial;
                trial.sheets.push_back({options.sheetWidth,options.sheetHeight,{}});
                trial.occupied.emplace_back();
                auto& sheet=trial.sheets.front();
                auto& occupied=trial.occupied.front();
                for (int copy=0;copy<high;++copy)
                    if (!place(part,static_cast<std::uint32_t>(copy),sheet,occupied,options,bottomFirst,gridOnly,allowed,leftAligned))
                        break;
                const bool simple=leftAligned || gridOnly;
                if (winner.sheets.empty() || std::make_pair(occupied.size(),simple)>
                    std::make_pair(winner.occupied.front().size(),winnerSimple) ||
                    (occupied.size()==winner.occupied.front().size() && simple==winnerSimple &&
                        trialScore(trial)<trialScore(winner)))
                {
                    winner=std::move(trial);
                    winnerSimple=simple;
                }
            };
            if (rotations.front().round)
            {
                part.quantity=high;
                auto bands=bandsFor(part,rotations,options,true);
                const auto score=[&](const Band& band)
                {
                    const double length=options.sheetHeight>=options.sheetWidth ? band.height : band.width;
                    return std::make_tuple(band.placements.size(),!band.staggered,-length,-band.width*band.height);
                };
                const auto selected=std::max_element(bands.begin(),bands.end(),
                    [&](const Band& a,const Band& b) { return score(a)<score(b); });
                if (selected==bands.end()) return best;
                winner.sheets.push_back({options.sheetWidth,options.sheetHeight,std::move(selected->placements)});
                winner.occupied.emplace_back();
                for (auto& placement:winner.sheets.front().placements)
                {
                    placement.transform.x+=options.margin;
                    placement.transform.y+=options.margin;
                    auto polygon=transformed(part.outline,placement.transform);
                    const auto box=bounds(polygon);
                    auto collision=CollisionShape(polygon);
                    winner.occupied.front().push_back({std::move(polygon),box,std::move(collision),true});
                }
            }
            else
            {
                fill(rotations,true,false,true);
                for (size_t i=0;i<rotations.size();++i)
                    for (size_t j=i+1;j<rotations.size();++j)
                        if (std::abs(std::abs(std::remainder(rotations[i].angle-rotations[j].angle,360.0))-180.0)<=tolerance)
                        {
                            const std::vector<Variant> aligned{rotations[i],rotations[j]};
                            fill(aligned,false,true,false);
                            if (winner.occupied.front().size()<static_cast<size_t>(high)) fill(aligned,true,false,false);
                        }
                if (winner.occupied.front().size()<static_cast<size_t>(high)) fill(rotations,false,false,false);
                if (winner.occupied.front().size()<static_cast<size_t>(high)) fill(rotations,true,false,false);
            }

            const auto& placements=winner.sheets.front().placements;
            if (placements.size()<=1) return best;
            part.quantity=static_cast<int>(placements.size());
            if (!validateLayout({part},winner.sheets,options.clearance,options.margin).empty()) return best;
            auto& pieces=best.layout.sheets.front().placed;
            pieces.clear();
            for (size_t i=0;i<placements.size();++i)
            {
                const auto& placement=placements[i];
                const auto& occupied=winner.occupied.front()[i];
                const auto& box=occupied.box;
                PlacedRect piece{box.left,box.top,box.right-box.left,box.bottom-box.top,false,part.id,placement.instanceIndex};
                piece.contour=std::make_shared<const Polygon>(occupied.polygon);
                piece.contourTransform=placement.transform;
                pieces.push_back(std::move(piece));
            }
            return best;
        }
        while (low<high)
        {
            part.quantity=low+(high-low+1)/2;
            auto trial=calculate({part},options);
            if (trial.layout.isValid() && trial.layout.sheets.size()==1 &&
                trial.layout.sheets.front().placed.size()==static_cast<size_t>(part.quantity))
            {
                low=part.quantity;
                best=std::move(trial);
            }
            else high=part.quantity-1;
        }
        return best;
    }

    Result calculate(const std::vector<NestingGeometry::Part>& parts, const Options& options) try
    {
        using namespace NestingGeometry;
        Result result;
        auto fail = [&](const std::string& message) { result.layout.validationErrors.push_back(message); };
        if (!std::isfinite(options.sheetWidth)||!std::isfinite(options.sheetHeight)||options.sheetWidth<=0||options.sheetHeight<=0||
            !std::isfinite(options.clearance)||options.clearance<0||!std::isfinite(options.margin)||options.margin<0 ||
            options.sheetWidth<=2*options.margin || options.sheetHeight<=2*options.margin)
        { fail("Enter positive sheet dimensions and valid clearance/margins."); return result; }
        int total=0;
        std::set<PartId> ids;
        bool rectangleCompatible=options.clearance==0 && options.margin==0;
        for (const auto& part:parts)
        {
            if (part.id==0 || !ids.insert(part.id).second) fail("Part identities must be non-zero and unique.");
            const auto errors=validateOutline(part.outline);
            result.layout.validationErrors.insert(result.layout.validationErrors.end(),errors.begin(),errors.end());
            if (part.quantity<0 || part.quantity>2000 || total>2000-part.quantity) fail("Use up to 2,000 pieces total.");
            else total+=part.quantity;
            if (part.allowedRotations.empty()) fail("Each shape needs an allowed rotation.");
            for (double angle:part.allowedRotations) if (!std::isfinite(angle)) fail("Allowed rotations must be finite.");
            rectangleCompatible=rectangleCompatible && allows(part,0) && allows(part,90);
        }
        if (!result.layout.validationErrors.empty()) return result;
        if (total==0) { fail("Add at least one piece before calculating."); return result; }
        result.method=methodFor(parts);
        if (!rectangleCompatible) result.method=Method::Contours;
        if (result.method==Method::Rectangles)
        {
            std::vector<Rect> rectangles;
            for (const auto& part:parts)
            {
                const auto box=bounds(part.outline);
                rectangles.push_back({box.right-box.left,box.bottom-box.top,part.quantity,part.id});
            }
            result.layout=NestingLayoutSearch::findBest(rectangles,options.sheetWidth,options.sheetHeight);
            for (auto& sheet:result.layout.sheets) for (auto& placed:sheet.placed)
            {
                const auto source=std::find_if(parts.begin(),parts.end(),[&](const Part& p) { return p.id==placed.partId; });
                const auto box=bounds(source->outline);
                if (std::abs(box.left)>tolerance || std::abs(box.top)>tolerance)
                {
                    placed.contourTransform=placed.rotated ? Transform{placed.x+box.bottom,placed.y-box.left,90}
                        : Transform{placed.x-box.left,placed.y-box.top,0};
                    placed.contour=std::make_shared<const Polygon>(transformed(source->outline,placed.contourTransform));
                }
            }
            return result;
        }
        if (total>200) { fail("Contour jobs support up to 200 pieces per calculation."); return result; }
        for (const auto& part:parts)
        {
            if (part.outline.holes.size()>16) { fail("Use up to 16 holes per profile."); return result; }
            for (const auto* boundary:boundaryRings(part.outline))
                if (boundary->vertices.size()>64) { fail("Use up to 64 vertices per boundary ring."); return result; }
        }
        Trial best;
        bool haveBest=false;
        const auto variants=prepareVariants(parts);
        for (int order=0;order<3;++order) for (bool bottomFirst:{false,true})
        {
            auto candidate=contourTrial(parts,options,variants,order,bottomFirst);
            if (candidate.complete && (!haveBest || trialScore(candidate)<trialScore(best)))
            { best=std::move(candidate); haveBest=true; }
        }
        // Compare a regular edge-aligned layout as well: locally tighter
        // staggered circles can otherwise prevent a later row from fitting.
        auto grid=contourTrial(parts,options,variants,0,true,true);
        if (grid.complete && (!haveBest || trialScore(grid)<trialScore(best)))
        { best=std::move(grid); haveBest=true; }
        if (parts.size()==1 && parts.front().quantity>=3 &&
            triangleLike(parts.front().outline))
        {
            const auto id=parts.front().id;
            const auto& rotations=variants.at(id);
            for (size_t i=0;i<rotations.size();++i)
                for (size_t j=i+1;j<rotations.size();++j)
                {
                    const double difference=std::remainder(rotations[i].angle-rotations[j].angle,360.0);
                    if (std::abs(std::abs(difference)-180.0)>tolerance) continue;
                    // Try a common rotation axis for the entire run of triangles.
                    const Variants aligned{{id,{rotations[i],rotations[j]}}};
                    for (bool bottomFirst:{false,true})
                    {
                        auto candidate=contourTrial(parts,options,aligned,0,bottomFirst);
                        if (candidate.complete && (!haveBest || trialScore(candidate)<trialScore(best)))
                        { best=std::move(candidate); haveBest=true; }
                    }
                }
        }
        for (bool vertical:{false,true})
        {
            auto structured=structuredTrial(parts,options,variants,vertical);
            if (structured.complete && (!haveBest || trialScore(structured)<trialScore(best)))
            { best=std::move(structured); haveBest=true; }
        }
        const auto pairs=NestingPairSearch::find(parts,variants,options.sheetWidth-2*options.margin,
            options.sheetHeight-2*options.margin,options.clearance);
        std::array<std::set<GroupPattern>,2> visited;
        for (int pairingOrder=0;pairingOrder<3;++pairingOrder) for (bool bottomFirst:{false,true})
        {
            auto grouped=groupedTrial(parts,options,variants,pairs,pairingOrder,bottomFirst,
                haveBest ? best.sheets.size() : 200,visited[bottomFirst ? 1 : 0]);
            if (grouped.complete && (!haveBest || trialScore(grouped)<trialScore(best)))
            { best=std::move(grouped); haveBest=true; }
        }
        if (!haveBest) { fail("A shape does not fit the sheet at its allowed rotations."); return result; }
        result.layout.validationErrors=validateLayout(parts,best.sheets,options.clearance,options.margin);
        if (!result.layout.validationErrors.empty()) return result;
        std::map<PartId,const Part*> definitions;
        for (const auto& part:parts) definitions.emplace(part.id,&part);
        for (const auto& sheet:best.sheets)
        {
            Sheet output; output.width=sheet.width; output.height=sheet.height;
            for (const auto& placement:sheet.placements)
            {
                auto polygon=transformed(definitions.at(placement.partId)->outline,placement.transform);
                const auto box=bounds(polygon);
                PlacedRect item{box.left,box.top,box.right-box.left,box.bottom-box.top,false,placement.partId,placement.instanceIndex};
                item.contour=std::make_shared<const Polygon>(std::move(polygon));
                item.contourTransform=placement.transform;
                output.placed.push_back(std::move(item));
            }
            result.layout.sheets.push_back(std::move(output));
        }
        return result;
    }
    catch (const std::exception& error)
    {
        Result failed;
        failed.layout.validationErrors.push_back(error.what());
        return failed;
    }
}
