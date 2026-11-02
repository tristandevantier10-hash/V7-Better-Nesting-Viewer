#include "NestingGeometry.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <limits>
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
        template<class A,class B> bool convexOverlap(const A& a,const B& b)
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
                    { const auto t=nx*(v.x-p.x)+ny*(v.y-p.y); bMin=std::min(bMin,t); bMax=std::max(bMax,t); }
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
        return {};
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
    double area(const Polygon& polygon) { requireValid(polygon); return static_cast<double>(std::abs(signedArea(ring(polygon)))); }
    Polygon transformed(const Polygon& polygon, const Transform& transform)
    {
        requireValid(polygon);
        if (!std::isfinite(transform.x)||!std::isfinite(transform.y)||!std::isfinite(transform.rotationDegrees))
            throw std::invalid_argument("Placement transforms must be finite.");
        const double angle=std::remainder(transform.rotationDegrees,360.0)*3.14159265358979323846/180.0;
        const double cosine=std::cos(angle), sine=std::sin(angle);
        Polygon result;
        for (const auto& p:polygon.vertices)
            result.vertices.push_back({p.x*cosine-p.y*sine+transform.x,p.x*sine+p.y*cosine+transform.y});
        requireValid(result);
        return result;
    }
    bool isRectangle(const Polygon& polygon)
    {
        if (!validateOutline(polygon).empty()) return false;
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
        requireValid(a); requireValid(b);
        if (!std::isfinite(clearance)||clearance<0) throw std::invalid_argument("Clearance must be finite and non-negative.");
        const auto ab=bounds(a), bb=bounds(b);
        if (ab.right+clearance<bb.left-tolerance || bb.right+clearance<ab.left-tolerance ||
            ab.bottom+clearance<bb.top-tolerance || bb.bottom+clearance<ab.top-tolerance) return false;
        const auto ar=ring(a), br=ring(b);
        if (convex(ar) && convex(br))
        { if (convexOverlap(ar,br)) return true; }
        else
        {
            const auto at=triangulate(a),bt=triangulate(b);
            for (const auto& x:at) for (const auto& y:bt) if (convexOverlap(x,y)) return true;
        }
        if (clearance==0) return false;
        for (size_t i=0;i<ar.size();++i) for (size_t j=0;j<br.size();++j)
        {
            const Point p=ar[i],q=ar[(i+1)%ar.size()],r=br[j],s=br[(j+1)%br.size()];
            if (segmentsMeet(p,q,r,s) || std::min({pointDistance(p,r,s),pointDistance(q,r,s),
                pointDistance(r,p,q),pointDistance(s,p,q)}) < clearance-tolerance) return true;
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
