#include "NestingGeometry.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <limits>
#include <iterator>
#include <map>
#include <set>
#include <stdexcept>

namespace NestingGeometry
{
    namespace
    {
        using Triangle = std::array<Point, 3>;
        long double cross(Point a, Point b, Point c)
        { return (static_cast<long double>(b.x) - a.x) * (c.y - a.y) -
            (static_cast<long double>(b.y) - a.y) * (c.x - a.x); }
        double length(Point a, Point b) { return std::hypot(b.x - a.x, b.y - a.y); }
        bool same(Point a, Point b) { return length(a,b) <= tolerance; }
        bool onSegment(Point p, Point a, Point b)
        {
            return std::abs(cross(a,b,p)) <= tolerance * std::max(1.0,length(a,b)) &&
                p.x >= std::min(a.x,b.x) - tolerance && p.x <= std::max(a.x,b.x) + tolerance &&
                p.y >= std::min(a.y,b.y) - tolerance && p.y <= std::max(a.y,b.y) + tolerance;
        }
        int side(Point a, Point b, Point p)
        {
            const auto value = cross(a,b,p);
            const auto eps = tolerance * std::max(1.0,length(a,b));
            return value > eps ? 1 : value < -eps ? -1 : 0;
        }
        bool segmentsMeet(Point a, Point b, Point c, Point d)
        {
            const int ac = side(a,b,c), ad = side(a,b,d), ca = side(c,d,a), cb = side(c,d,b);
            return (ac * ad < 0 && ca * cb < 0) || onSegment(c,a,b) ||
                onSegment(d,a,b) || onSegment(a,c,d) || onSegment(b,c,d);
        }
        std::vector<Point> ring(const Polygon& polygon)
        {
            auto points = polygon.vertices;
            if (points.size() > 1 && same(points.front(),points.back())) points.pop_back();
            return points;
        }
        bool inside(Point p, const std::vector<Point>& points)
        {
            bool result=false;
            for (size_t i=0;i<points.size();++i)
            {
                const auto a=points[i], b=points[(i+1)%points.size()];
                if (((a.y>p.y)!=(b.y>p.y)) && p.x<a.x+(p.y-a.y)*(b.x-a.x)/(b.y-a.y)) result=!result;
            }
            return result;
        }
        RingRelation relation(const Polygon& a, const Polygon& b)
        {
            const auto ar=ring(a),br=ring(b);
            for (size_t i=0;i<ar.size();++i) for (size_t j=0;j<br.size();++j)
                if (segmentsMeet(ar[i],ar[(i+1)%ar.size()],br[j],br[(j+1)%br.size()]))
                    return RingRelation::Intersecting;
            if (inside(br.front(),ar)) return RingRelation::Contains;
            if (inside(ar.front(),br)) return RingRelation::Inside;
            return RingRelation::Separate;
        }
        long double signedArea(const std::vector<Point>& points)
        {
            long double result = 0;
            if (points.empty()) return result;
            // Translate to the first vertex to reduce cancellation far from zero.
            for (size_t i = 1; i + 1 < points.size(); ++i)
                result += cross(points.front(),points[i],points[i+1]);
            return result / 2;
        }
        void requireValid(const Polygon& polygon)
        {
            const auto errors = validateOutline(polygon);
            if (!errors.empty()) throw std::invalid_argument(errors.front());
        }
        bool inTriangle(Point p, const Triangle& t)
        { return side(t[0],t[1],p) >= 0 && side(t[1],t[2],p) >= 0 && side(t[2],t[0],p) >= 0; }
        std::vector<Triangle> triangulate(const Polygon& polygon)
        {
            if (!polygon.holes.empty())
            {
                // Between consecutive vertex Y coordinates no edges cross or
                // change order. Pair crossings across ALL rings to decompose
                // material into trapezoids, leaving the hole interiors empty.
                struct Edge { Point a,b; double x(double y) const
                    { return a.x+(y-a.y)*(b.x-a.x)/(b.y-a.y); } };
                std::vector<Edge> edges;
                std::set<double> levels;
                for (const auto* boundary:boundaryRings(polygon))
                {
                    const auto points=ring(*boundary);
                    for (size_t i=0;i<points.size();++i)
                    {
                        const auto a=points[i],b=points[(i+1)%points.size()];
                        levels.insert(a.y);
                        if (a.y!=b.y) edges.push_back({a,b});
                    }
                }
                std::vector<Triangle> result;
                auto add=[&](Point a,Point b,Point c)
                {
                    if (std::abs(cross(a,b,c))>tolerance*std::max({length(a,b),length(b,c),length(c,a)}))
                        result.push_back({a,b,c});
                };
                for (auto low=levels.begin(),high=std::next(low);high!=levels.end();++low,++high)
                {
                    if (*high-*low<=tolerance) continue; // Quarter-turn roundoff can split one level.
                    const double mid=*low+(*high-*low)/2;
                    std::vector<const Edge*> active;
                    for (const auto& edge:edges)
                        if (mid>std::min(edge.a.y,edge.b.y) && mid<std::max(edge.a.y,edge.b.y)) active.push_back(&edge);
                    std::sort(active.begin(),active.end(),[&](const Edge* a,const Edge* b) { return a->x(mid)<b->x(mid); });
                    if (active.size()%2) throw std::invalid_argument("Invalid material boundary crossings.");
                    for (size_t i=0;i+1<active.size();i+=2)
                    {
                        const Point a{active[i]->x(*low),*low},b{active[i+1]->x(*low),*low};
                        const Point c{active[i+1]->x(*high),*high},d{active[i]->x(*high),*high};
                        add(a,b,c); add(a,c,d);
                    }
                }
                return result;
            }
            auto points = ring(polygon);
            if (signedArea(points) < 0) std::reverse(points.begin(),points.end());
            // Remove redundant collinear vertices before ear clipping.
            bool changed = true;
            while (changed && points.size() > 3)
            {
                changed = false;
                for (size_t i = 0; i < points.size(); ++i)
                    if (onSegment(points[i],points[(i+points.size()-1)%points.size()],points[(i+1)%points.size()]))
                    { points.erase(points.begin()+i); changed = true; break; }
            }
            std::vector<Triangle> result;
            while (points.size() > 3)
            {
                bool removed = false;
                for (size_t i = 0; i < points.size(); ++i)
                {
                    const size_t previous = (i+points.size()-1)%points.size(), next = (i+1)%points.size();
                    Triangle ear{points[previous],points[i],points[next]};
                    if (side(ear[0],ear[1],ear[2]) <= 0) continue;
                    bool occupied = false;
                    for (size_t j = 0; j < points.size(); ++j)
                        if (j != previous && j != i && j != next && inTriangle(points[j],ear))
                        { occupied = true; break; }
                    if (occupied) continue;
                    result.push_back(ear);
                    points.erase(points.begin()+i);
                    removed = true;
                    break;
                }
                if (!removed) throw std::invalid_argument("Outline cannot be triangulated at the configured tolerance.");
            }
            result.push_back({points[0],points[1],points[2]});
            return result;
        }
        bool convex(const std::vector<Point>& points)
        {
            int direction=0;
            for (size_t i=0;i<points.size();++i)
            {
                const int current=side(points[i],points[(i+1)%points.size()],points[(i+2)%points.size()]);
                if (current==0) continue;
                if (direction!=0 && current!=direction) return false;
                direction=current;
            }
            return direction!=0;
        }
        template<class A,class B> bool convexOverlap(const A& a,const B& b,Point offset)
        {
            auto separated=[&](const auto& polygon)
            {
                for (size_t edge=0;edge<polygon.size();++edge)
                {
                    const Point p=polygon[edge],q=polygon[(edge+1)%polygon.size()];
                    const long double nx=-(q.y-p.y),ny=q.x-p.x;
                    long double aMin=std::numeric_limits<long double>::max(),aMax=-aMin;
                    long double bMin=aMin,bMax=-aMin;
                    for (const auto& v:a)
                    { const auto t=nx*(v.x-p.x)+ny*(v.y-p.y); aMin=std::min(aMin,t); aMax=std::max(aMax,t); }
                    for (const auto& v:b)
                    { const auto t=nx*(v.x+offset.x-p.x)+ny*(v.y+offset.y-p.y); bMin=std::min(bMin,t); bMax=std::max(bMax,t); }
                    if (std::min(aMax,bMax)-std::max(aMin,bMin)<=tolerance*length(p,q)) return true;
                }
                return false;
            };
            return !separated(a) && !separated(b);
        }
        double pointDistance(Point p, Point a, Point b)
        {
            const double dx=b.x-a.x, dy=b.y-a.y;
            const double t=std::clamp(((p.x-a.x)*dx+(p.y-a.y)*dy)/(dx*dx+dy*dy),0.0,1.0);
            return length(p,{a.x+t*dx,a.y+t*dy});
        }
    }

    PartId newPartId()
    {
        static std::atomic<PartId> next{1};
        return next.fetch_add(1,std::memory_order_relaxed);
    }

    std::vector<std::string> validateOutline(const Polygon& polygon)
    {
        for (const auto& p : polygon.vertices)
            if (!std::isfinite(p.x) || !std::isfinite(p.y) || std::abs(p.x)>1e9 || std::abs(p.y)>1e9)
                return {"Outline coordinates must be finite and within one billion millimetres of the origin."};
        const auto points = ring(polygon);
        if (points.size() < 3) return {"An outline needs at least three distinct vertices."};
        for (size_t i=0;i<points.size();++i)
        {
            const Point a=points[i], b=points[(i+1)%points.size()];
            if (same(a,b)) return {"An outline cannot have zero-length edges."};
            const Point c=points[(i+2)%points.size()];
            if (side(a,b,c)==0 && !onSegment(b,a,c)) return {"An outline cannot double back along an edge."};
            for (size_t j=i+1;j<points.size();++j)
            {
                if (j==i+1 || (i==0 && j+1==points.size())) continue;
                if (segmentsMeet(a,b,points[j],points[(j+1)%points.size()]))
                    return {"An outline cannot self-intersect or touch itself."};
            }
        }
        if (std::abs(signedArea(points)) <= tolerance*tolerance) return {"An outline must enclose positive area."};
        const Polygon outer{polygon.vertices};
        for (size_t i=0;i<polygon.holes.size();++i)
        {
            const auto& hole=polygon.holes[i];
            if (!hole.holes.empty()) return {"Islands must be separate parts, not holes within holes."};
            const auto errors=validateOutline(hole);
            if (!errors.empty()) return errors;
            if (relation(outer,hole)!=RingRelation::Contains)
                return {"Holes must lie strictly inside the outer outline without touching it."};
            for (size_t j=0;j<i;++j)
                if (relation(hole,polygon.holes[j])!=RingRelation::Separate)
                    return {"Holes cannot overlap, touch or contain each other."};
        }
        return {};
    }

    RingRelation ringRelation(const Polygon& a,const Polygon& b)
    {
        requireValid(a); requireValid(b);
        if (!a.holes.empty() || !b.holes.empty()) throw std::invalid_argument("Ring classification requires simple rings.");
        return relation(a,b);
    }
    std::vector<const Polygon*> boundaryRings(const Polygon& polygon)
    {
        std::vector<const Polygon*> result{&polygon};
        for (const auto& hole:polygon.holes) result.push_back(&hole);
        return result;
    }
    std::vector<double> scanline(const Polygon& polygon,double y)
    {
        std::vector<double> crossings;
        for (const auto* boundary:boundaryRings(polygon))
            for (size_t i=0;i<boundary->vertices.size();++i)
            {
                const auto a=boundary->vertices[i],b=boundary->vertices[(i+1)%boundary->vertices.size()];
                if ((a.y<=y && b.y>y)||(b.y<=y && a.y>y))
                    crossings.push_back(a.x+(y-a.y)*(b.x-a.x)/(b.y-a.y));
            }
        std::sort(crossings.begin(),crossings.end());
        return crossings;
    }

    Polygon rectangle(double width, double height)
    {
        if (!std::isfinite(width) || !std::isfinite(height) || width<=0 || height<=0)
            throw std::invalid_argument("Rectangle dimensions must be finite and positive.");
        return {{{0,0},{width,0},{width,height},{0,height}}};
    }
    Polygon circle(double diameter)
    {
        if (!std::isfinite(diameter)||diameter<=0) throw std::invalid_argument("Circle diameter must be positive.");
        constexpr int segments=64;
        constexpr double pi=3.14159265358979323846;
        const double radius=diameter*0.5/std::cos(pi/segments);
        Polygon result;
        for (int i=0;i<segments;++i)
        {
            const double angle=(2*i+1)*pi/segments;
            result.vertices.push_back({diameter*0.5+radius*std::cos(angle),diameter*0.5+radius*std::sin(angle)});
        }
        return result;
    }
    Bounds bounds(const Polygon& polygon)
    {
        requireValid(polygon);
        Bounds result{polygon.vertices.front().x,polygon.vertices.front().y,
            polygon.vertices.front().x,polygon.vertices.front().y};
        for (const auto& p:polygon.vertices)
        { result.left=std::min(result.left,p.x); result.top=std::min(result.top,p.y);
          result.right=std::max(result.right,p.x); result.bottom=std::max(result.bottom,p.y); }
        return result;
    }
    double area(const Polygon& polygon)
    {
        requireValid(polygon);
        long double result=std::abs(signedArea(ring(polygon)));
        for (const auto& hole:polygon.holes) result-=std::abs(signedArea(ring(hole)));
        return static_cast<double>(result);
    }
    Polygon transformed(const Polygon& polygon, const Transform& transform)
    {
        requireValid(polygon);
        if (!std::isfinite(transform.x)||!std::isfinite(transform.y)||!std::isfinite(transform.rotationDegrees))
            throw std::invalid_argument("Placement transforms must be finite.");
        const double angle=std::remainder(transform.rotationDegrees,360.0)*3.14159265358979323846/180.0;
        const double cosine=std::cos(angle), sine=std::sin(angle);
        Polygon result=polygon;
        mapVertices(result,[&](Point p) { return Point{p.x*cosine-p.y*sine+transform.x,p.x*sine+p.y*cosine+transform.y}; });
        requireValid(result);
        return result;
    }
    bool isRectangle(const Polygon& polygon)
    {
        if (!polygon.holes.empty() || !validateOutline(polygon).empty()) return false;
        const auto box=bounds(polygon);
        const double expected=(box.right-box.left)*(box.bottom-box.top);
        const double scale=std::max(1.0,std::max(box.right-box.left,box.bottom-box.top));
        return std::abs(area(polygon)-expected)<=tolerance*scale;
    }
    Method methodFor(const std::vector<Part>& parts)
    {
        Method method = Method::Rectangles;
        for (const auto& part:parts)
        { requireValid(part.outline); if (!isRectangle(part.outline)) method = Method::Contours; }
        return method;
    }

    bool collide(const Polygon& a, const Polygon& b, double clearance)
    {
        return collide(CollisionShape(a),{},CollisionShape(b),{},clearance);
    }
    CollisionShape::CollisionShape(const Polygon& polygon)
    {
        requireValid(polygon);
        auto pointBounds=[](const std::vector<Point>& points)
        {
            Bounds result{points.front().x,points.front().y,points.front().x,points.front().y};
            for (const auto& p:points)
            { result.left=std::min(result.left,p.x); result.right=std::max(result.right,p.x);
              result.top=std::min(result.top,p.y); result.bottom=std::max(result.bottom,p.y); }
            return result;
        };
        for (const auto* boundary:boundaryRings(polygon)) boundaries.push_back(ring(*boundary));
        box=pointBounds(boundaries.front());
        // Keep convex disks as one SAT cell, even when the OTHER part has holes.
        // Triangulating both sides needlessly multiplies narrow-phase work.
        if (polygon.holes.empty() && convex(boundaries.front())) cells.push_back({boundaries.front(),box});
        else
            for (const auto& triangle:triangulate(polygon))
            {
                std::vector<Point> points(triangle.begin(),triangle.end());
                const auto cellBox=pointBounds(points);
                cells.push_back({std::move(points),cellBox});
            }
    }
    bool collide(const CollisionShape& a,Point offsetA,const CollisionShape& b,Point offsetB,double clearance)
    {
        if (!std::isfinite(clearance)||clearance<0) throw std::invalid_argument("Clearance must be finite and non-negative.");
        if (!std::isfinite(offsetA.x)||!std::isfinite(offsetA.y)||!std::isfinite(offsetB.x)||!std::isfinite(offsetB.y))
            throw std::invalid_argument("Collision offsets must be finite.");
        auto validPosition=[](const Bounds& box,Point offset)
        {
            return std::abs(box.left+offset.x)<=1e9 && std::abs(box.right+offset.x)<=1e9 &&
                std::abs(box.top+offset.y)<=1e9 && std::abs(box.bottom+offset.y)<=1e9;
        };
        if (!validPosition(a.box,offsetA) || !validPosition(b.box,offsetB))
            throw std::invalid_argument("Translated collision coordinates must remain within one billion millimetres of the origin.");
        const Point delta{offsetB.x-offsetA.x,offsetB.y-offsetA.y};
        const auto& ab=a.box; const auto& bb=b.box;
        if (ab.right+clearance<bb.left+delta.x-tolerance || bb.right+delta.x+clearance<ab.left-tolerance ||
            ab.bottom+clearance<bb.top+delta.y-tolerance || bb.bottom+delta.y+clearance<ab.top-tolerance) return false;
        for (const auto& ac:a.cells) for (const auto& bc:b.cells)
        {
            if (ac.box.right<=bc.box.left+delta.x+tolerance || bc.box.right+delta.x<=ac.box.left+tolerance ||
                ac.box.bottom<=bc.box.top+delta.y+tolerance || bc.box.bottom+delta.y<=ac.box.top+tolerance) continue;
            if (convexOverlap(ac.vertices,bc.vertices,delta)) return true;
        }
        if (clearance==0) return false;
        for (const auto& ar:a.boundaries) for (const auto& br:b.boundaries)
        {
            for (size_t i=0;i<ar.size();++i) for (size_t j=0;j<br.size();++j)
            {
                const Point p=ar[i],q=ar[(i+1)%ar.size()];
                const Point r{br[j].x+delta.x,br[j].y+delta.y};
                const auto next=br[(j+1)%br.size()];
                const Point s{next.x+delta.x,next.y+delta.y};
                if (segmentsMeet(p,q,r,s) || std::min({pointDistance(p,r,s),pointDistance(q,r,s),
                    pointDistance(r,p,q),pointDistance(s,p,q)}) < clearance-tolerance) return true;
            }
        }
        return false;
    }
    bool fitsSheet(const Polygon& polygon, double width, double height, double margin)
    {
        if (!std::isfinite(width)||!std::isfinite(height)||!std::isfinite(margin)||width<=0||height<=0||margin<0)
            throw std::invalid_argument("Sheet dimensions must be positive and margin non-negative.");
        const auto box=bounds(polygon);
        return box.left>=margin-tolerance && box.top>=margin-tolerance &&
            box.right<=width-margin+tolerance && box.bottom<=height-margin+tolerance;
    }
    std::vector<std::string> validateLayout(const std::vector<Part>& parts,
        const std::vector<SheetLayout>& sheets, double clearance, double margin)
    {
        std::vector<std::string> errors;
        if (!std::isfinite(clearance)||clearance<0||!std::isfinite(margin)||margin<0)
            return {"Clearance and margin must be finite and non-negative."};
        std::map<PartId,const Part*> definitions;
        for (const auto& part:parts)
        {
            if (part.id==0 || !definitions.emplace(part.id,&part).second)
                errors.push_back("Part identities must be non-zero and unique.");
            if (part.quantity<0) errors.push_back("Part quantities cannot be negative.");
            const auto outlineErrors=validateOutline(part.outline);
            errors.insert(errors.end(),outlineErrors.begin(),outlineErrors.end());
            if (part.allowedRotations.empty()) errors.push_back("A part needs at least one allowed rotation.");
            for (double angle:part.allowedRotations)
                if (!std::isfinite(angle)) errors.push_back("Allowed rotations must be finite.");
        }
        if (!errors.empty()) return errors;
        std::set<std::pair<PartId,std::uint32_t>> seen;
        for (const auto& sheet:sheets)
        {
            if (!std::isfinite(sheet.width)||!std::isfinite(sheet.height)||sheet.width<=0||sheet.height<=0)
            { errors.push_back("Sheet dimensions must be finite and positive."); continue; }
            std::vector<Polygon> outlines;
            for (const auto& placement:sheet.placements)
            {
                const auto found=definitions.find(placement.partId);
                if (found==definitions.end()) { errors.push_back("Placement references an unknown part."); continue; }
                const auto& part=*found->second;
                if (placement.instanceIndex>=static_cast<std::uint32_t>(part.quantity) ||
                    !seen.emplace(placement.partId,placement.instanceIndex).second)
                    errors.push_back("A part instance is duplicated or outside its requested quantity.");
                bool allowed=false;
                for (double angle:part.allowedRotations)
                    if (std::abs(std::remainder(placement.transform.rotationDegrees-angle,360.0))<=tolerance) allowed=true;
                if (!allowed) errors.push_back("Placement uses a disallowed rotation.");
                try
                {
                    auto outline=transformed(part.outline,placement.transform);
                    if (!fitsSheet(outline,sheet.width,sheet.height,margin)) errors.push_back("Part lies outside the usable sheet.");
                    for (const auto& previous:outlines)
                        if (collide(outline,previous,clearance)) errors.push_back("Part contours overlap or violate clearance.");
                    outlines.push_back(std::move(outline));
                }
                catch (const std::exception& error) { errors.push_back(error.what()); }
            }
        }
        for (const auto& part:parts)
            for (int instance=0;instance<part.quantity;++instance)
                if (seen.count({part.id,static_cast<std::uint32_t>(instance)})==0)
                    errors.push_back("A requested part instance is missing.");
        return errors;
    }
}
