#include "NestingService.h"
#include "NestingLayoutSearch.h"
#include <algorithm>
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
        struct Occupied { Polygon polygon; Bounds box; CollisionShape collision; };
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
        bool place(const Part& part, std::uint32_t instance, SheetLayout& sheet,
            std::vector<Occupied>& occupied, const Options& options, bool bottomFirst, bool gridOnly)
        {
            bool found = false;
            Transform bestTransform;
            Polygon bestPolygon;
            Bounds bestBounds;
            std::tuple<double,double,double,double> bestScore;
            double previousRight=options.margin, previousBottom=options.margin;
            for (const auto& p:occupied)
            { previousRight=std::max(previousRight,p.box.right); previousBottom=std::max(previousBottom,p.box.bottom); }
            for (double angle:part.allowedRotations)
            {
                const auto rotated = transformed(part.outline,{0,0,angle});
                const auto rb = bounds(rotated);
                const auto local = transformed(rotated,{-rb.left,-rb.top,0});
                const CollisionShape localCollision(local);
                const double width=rb.right-rb.left, height=rb.bottom-rb.top;
                const bool round=circleEnvelope(rotated,rb);
                if (width > sheet.width-2*options.margin+tolerance || height > sheet.height-2*options.margin+tolerance) continue;
                std::set<std::pair<double,double>> anchors;
                auto add = [&](double x,double y)
                {
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
                for (const auto& previous:occupied)
                {
                    if (round && circleEnvelope(previous.polygon,previous.box))
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
            occupied.push_back({std::move(bestPolygon),bestBounds,std::move(collision)});
            return true;
        }
        Trial contourTrial(const std::vector<Part>& parts, const Options& options, int order, bool bottomFirst, bool gridOnly=false)
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
                        if (place(parts[index],static_cast<std::uint32_t>(copy),result.sheets[sheet],result.occupied[sheet],options,bottomFirst,gridOnly))
                        { placed=true; break; }
                    if (!placed)
                    {
                        result.sheets.push_back({options.sheetWidth,options.sheetHeight,{}});
                        result.occupied.emplace_back();
                        if (!place(parts[index],static_cast<std::uint32_t>(copy),result.sheets.back(),result.occupied.back(),options,bottomFirst,gridOnly))
                        { result.complete=false; return result; }
                    }
                }
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
        for (int order=0;order<3;++order) for (bool bottomFirst:{false,true})
        {
            auto candidate=contourTrial(parts,options,order,bottomFirst);
            if (candidate.complete && (!haveBest || trialScore(candidate)<trialScore(best)))
            { best=std::move(candidate); haveBest=true; }
        }
        // Compare a regular edge-aligned layout as well: locally tighter
        // staggered circles can otherwise prevent a later row from fitting.
        auto grid=contourTrial(parts,options,0,true,true);
        if (grid.complete && (!haveBest || trialScore(grid)<trialScore(best)))
        { best=std::move(grid); haveBest=true; }
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
