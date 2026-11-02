#pragma once
#include "NestingGeometry.h"
#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <tuple>

// Geometry-only pair proposals shared by presets and imported profiles.
namespace NestingPairSearch
{
    using namespace NestingGeometry;
    struct Variant
    {
        Polygon local;
        Bounds rotatedBounds;
        CollisionShape collision;
        double angle;
        bool round;
    };
    using Variants=std::map<PartId,std::vector<Variant>>;
    struct Pair
    {
        size_t first=0,second=0;
        Transform a,b;
        double width=0,height=0,saving=0;
    };

    inline std::vector<Pair> find(const std::vector<Part>& parts,const Variants& variants,
        double usableWidth,double usableHeight,double clearance)
    {
        size_t combinations=0;
        for (size_t i=0;i<parts.size();++i) for (size_t j=i;j<parts.size();++j)
            if (parts[i].quantity>0 && parts[j].quantity>0 && (i!=j || parts[i].quantity>1))
                combinations+=variants.at(parts[i].id).size()*variants.at(parts[j].id).size();
        // Share the work budget across all source pairs and orientations. This
        // bounds the extra search without excluding a class of DXF geometry.
        const size_t attempts=std::clamp<size_t>(32768/std::max<size_t>(1,combinations),1,256);
        std::vector<Pair> result;
        for (size_t i=0;i<parts.size();++i) for (size_t j=i;j<parts.size();++j)
        {
            if (parts[i].quantity<=0 || parts[j].quantity<=0 || (i==j && parts[i].quantity<2)) continue;
            bool found=false;
            Pair best;
            std::tuple<double,double,double> bestScore;
            for (const auto& a:variants.at(parts[i].id)) for (const auto& b:variants.at(parts[j].id))
            {
                const double aw=a.rotatedBounds.right-a.rotatedBounds.left,ah=a.rotatedBounds.bottom-a.rotatedBounds.top;
                const double bw=b.rotatedBounds.right-b.rotatedBounds.left,bh=b.rotatedBounds.bottom-b.rotatedBounds.top;
                const double separate=aw*ah+bw*bh;
                std::set<std::pair<double,double>> anchors;
                auto add=[&](double x,double y)
                {
                    if (anchors.size()<2048) anchors.emplace(x,y);
                };
                for (double x:{-bw-clearance,0.0,aw-bw,aw+clearance,(aw-bw)/2})
                    for (double y:{-bh-clearance,0.0,ah-bh,ah+clearance,(ah-bh)/2}) add(x,y);
                for (const auto& hole:a.local.holes)
                {
                    const auto box=bounds(hole);
                    for (double x:{box.left+clearance,box.right-bw-clearance,(box.left+box.right-bw)/2})
                        for (double y:{box.top+clearance,box.bottom-bh-clearance,(box.top+box.bottom-bh)/2}) add(x,y);
                }
                for (const auto& hole:b.local.holes)
                {
                    const auto box=bounds(hole);
                    for (double x:{-box.left-clearance,aw-box.right+clearance,(aw-box.left-box.right)/2})
                        for (double y:{-box.top-clearance,ah-box.bottom+clearance,(ah-box.top-box.bottom)/2}) add(x,y);
                }
                // Reserve diagonal edge contacts before axis-offset vertex
                // contacts consume the bounded proposal set.
                for (const auto* ar:boundaryRings(a.local)) for (const auto* br:boundaryRings(b.local))
                    for (size_t ai=0;ai<ar->vertices.size() && anchors.size()<2048;++ai)
                    {
                        const auto p=ar->vertices[ai],end=ar->vertices[(ai+1)%ar->vertices.size()];
                        const double length=std::hypot(end.x-p.x,end.y-p.y);
                        if (length<=tolerance) continue;
                        const Point n{-(end.y-p.y)/length,(end.x-p.x)/length};
                        for (size_t bi=0;bi<br->vertices.size() && anchors.size()<2048;++bi)
                        {
                            const auto q=br->vertices[bi],next=br->vertices[(bi+1)%br->vertices.size()];
                            const double gap=clearance>0 ? clearance+10*tolerance : 0;
                            for (double sign:{-1.0,1.0})
                            {
                                add(p.x-q.x+sign*n.x*gap,p.y-q.y+sign*n.y*gap);
                                add((p.x+end.x-q.x-next.x)/2+sign*n.x*gap,
                                    (p.y+end.y-q.y-next.y)/2+sign*n.y*gap);
                            }
                        }
                    }
                if (anchors.size()<2048) for (const auto* ar:boundaryRings(a.local)) for (const auto* br:boundaryRings(b.local))
                    for (const auto& p:ar->vertices) for (const auto& q:br->vertices)
                        for (double dx:{-clearance,0.0,clearance}) for (double dy:{-clearance,0.0,clearance})
                            add(p.x-q.x+dx,p.y-q.y+dy);
                struct Candidate { double x,y,left,top,width,height; };
                std::vector<Candidate> candidates;
                for (const auto& anchor:anchors)
                {
                    const double x=anchor.first,y=anchor.second,left=std::min(0.0,x),top=std::min(0.0,y);
                    const double width=std::max(aw,x+bw)-left,height=std::max(ah,y+bh)-top;
                    if (width>usableWidth+tolerance || height>usableHeight+tolerance || width*height>=separate-tolerance) continue;
                    candidates.push_back({x,y,left,top,width,height});
                }
                std::stable_sort(candidates.begin(),candidates.end(),[](const Candidate& a,const Candidate& b)
                { return std::make_tuple(a.width*a.height,a.height,a.width,a.y,a.x)<
                    std::make_tuple(b.width*b.height,b.height,b.width,b.y,b.x); });
                size_t tested=0;
                // Finely faceted circles already have dedicated row/contact
                // strategies; retain generic mixed-shape probes without
                // spending the full irregular-profile budget on near-duplicates.
                const size_t limit=(a.round || b.round) ? std::min<size_t>(attempts,48) : attempts;
                for (const auto& c:candidates)
                {
                    if (tested++>=limit) break;
                    const auto score=std::make_tuple(c.width*c.height,c.height,c.width);
                    if (found && !(score<bestScore)) break;
                    if (collide(a.collision,{},b.collision,{c.x,c.y},clearance)) continue;
                    best={i,j,{-c.left-a.rotatedBounds.left,-c.top-a.rotatedBounds.top,a.angle},
                        {c.x-c.left-b.rotatedBounds.left,c.y-c.top-b.rotatedBounds.top,b.angle},
                        c.width,c.height,separate-c.width*c.height};
                    bestScore=score;
                    found=true;
                    break;
                }
            }
            if (found) result.push_back(best);
        }
        return result;
    }
}
