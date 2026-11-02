#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "DxfImport.h"
#include <windows.h>
#include <commdlg.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <limits>
#pragma comment(lib,"comdlg32.lib")

namespace DxfImport
{
    namespace
    {
        using namespace NestingGeometry;
        struct Group { int code; std::string value; };
        struct Entity { std::string type; std::vector<Group> groups; };
        std::string trim(std::string value)
        {
            const auto first=value.find_first_not_of(" \t\r\n");
            if (first==std::string::npos) return {};
            return value.substr(first,value.find_last_not_of(" \t\r\n")-first+1);
        }
        double number(const std::string& text)
        {
            size_t end=0;
            const double value=std::stod(text,&end);
            if (end!=text.size() || !std::isfinite(value)) throw std::runtime_error("DXF contains an invalid numeric value.");
            return value;
        }
        int integer(const std::string& text)
        {
            const double value=number(text);
            if (std::floor(value)!=value || value<std::numeric_limits<int>::min() || value>std::numeric_limits<int>::max())
                throw std::runtime_error("DXF contains an invalid integer group value.");
            return static_cast<int>(value);
        }
        std::string field(const Entity& entity,int code,const std::string& fallback={})
        {
            for (const auto& group:entity.groups) if (group.code==code) return group.value;
            return fallback;
        }
        double required(const Entity& entity,int code)
        {
            const auto value=field(entity,code);
            if (value.empty()) throw std::runtime_error("A "+entity.type+" entity is missing coordinates or dimensions.");
            return number(value);
        }
        double factor(int units,double unspecified)
        {
            switch (units)
            {
            case 0:return unspecified; case 1:return 25.4; case 2:return 304.8;
            case 3:return 1609344; case 4:return 1; case 5:return 10;
            case 6:return 1000; case 7:return 1000000; case 8:return 0.0000254;
            case 9:return 0.0254; case 10:return 914.4; case 11:return 0.0000001;
            case 12:return 0.000001; case 13:return 0.001; case 14:return 100;
            case 15:return 10000; case 16:return 100000;
            default:throw std::runtime_error("Unsupported DXF drawing units. Export this drawing in millimetres.");
            }
        }
        void require2d(const Entity& entity)
        {
            for (const auto& group:entity.groups)
            {
                if ((group.code==30 || group.code==31 || group.code==38 || group.code==39 || group.code==210 || group.code==220) &&
                    std::abs(number(group.value))>tolerance)
                    throw std::runtime_error("Only flat 2D outlines with standard Z extrusion are supported.");
                if (group.code==230 && std::abs(number(group.value)-1)>tolerance)
                    throw std::runtime_error("Non-standard DXF extrusion is not supported. Export a flat XY outline.");
                if (group.code==67 && integer(group.value)!=0)
                    throw std::runtime_error("Export cutting geometry from model space, not paper space.");
            }
        }
        void requireStraight(const Entity& entity)
        {
            require2d(entity);
            for (const auto& group:entity.groups)
                if ((group.code==42 || group.code==40 || group.code==41 || group.code==43) && std::abs(number(group.value))>tolerance)
                    throw std::runtime_error("Curved or wide polylines are not supported yet. Export a closed straight-segment outline.");
        }
        bool pointsNear(NestingGeometry::Point a,NestingGeometry::Point b) { return std::hypot(a.x-b.x,a.y-b.y)<=1e-6; }
        struct Segment { NestingGeometry::Point a,b; std::string layer; bool used=false; };
    }

    Result parse(const std::string& text,double unitlessToMillimetres)
    {
        Result result;
        try
        {
            if (text.size()>8*1024*1024) throw std::runtime_error("DXF files must be smaller than 8 MB.");
            if (text.find("AutoCAD Binary DXF")==0 || text.find('\0')!=std::string::npos)
                throw std::runtime_error("Binary DXF is not supported. Save as ASCII DXF.");
            if (!std::isfinite(unitlessToMillimetres)||unitlessToMillimetres<=0) throw std::runtime_error("Invalid DXF unit conversion.");
            std::istringstream stream(text);
            std::string code,value;
            std::vector<Group> groups;
            while (std::getline(stream,code))
            {
                if (groups.empty() && code.compare(0,3,"\xEF\xBB\xBF")==0) code.erase(0,3);
                code=trim(code);
                if (code.empty())
                {
                    if (stream.peek()==std::char_traits<char>::eof()) break;
                    throw std::runtime_error("DXF has a blank group-code line.");
                }
                if (!std::getline(stream,value)) throw std::runtime_error("DXF ends inside a group-code pair.");
                groups.push_back({integer(code),trim(value)});
            }
            bool eof=false, entitiesFound=false, sectionOpen=false;
            std::string section;
            int units=0;
            std::vector<Entity> entities;
            for (size_t i=0;i<groups.size();++i)
            {
                const auto& group=groups[i];
                if (group.code==0 && group.value=="SECTION")
                {
                    if (sectionOpen || i+1>=groups.size() || groups[i+1].code!=2) throw std::runtime_error("DXF has an invalid section header.");
                    section=groups[++i].value; sectionOpen=true;
                    if (section=="ENTITIES") entitiesFound=true;
                    continue;
                }
                if (group.code==0 && group.value=="ENDSEC")
                { if (!sectionOpen) throw std::runtime_error("DXF has an unmatched ENDSEC."); section.clear(); sectionOpen=false; continue; }
                if (group.code==0 && group.value=="EOF")
                { if (sectionOpen) throw std::runtime_error("DXF has an unclosed section."); eof=true; break; }
                if (section=="HEADER" && group.code==9 && group.value=="$INSUNITS")
                {
                    if (i+1>=groups.size() || groups[i+1].code!=70) throw std::runtime_error("DXF drawing units are malformed.");
                    units=integer(groups[++i].value);
                }
                if (section=="ENTITIES")
                {
                    if (group.code==0) entities.push_back({group.value,{}});
                    else if (entities.empty()) throw std::runtime_error("DXF entity attributes have no entity.");
                    else entities.back().groups.push_back(group);
                }
            }
            if (!eof || !entitiesFound) throw std::runtime_error("A complete ASCII DXF with an ENTITIES section is required.");
            result.unitless=units==0;
            const double scale=factor(units,unitlessToMillimetres);
            std::vector<Outline> raw;
            std::vector<Segment> segments;
            bool skippedAnnotations=false;
            auto add=[&](NestingGeometry::Polygon polygon,const std::string& name,bool circle=false)
            {
                if (polygon.vertices.size()>1 && pointsNear(polygon.vertices.front(),polygon.vertices.back())) polygon.vertices.pop_back();
                const auto errors=validateOutline(polygon);
                if (!errors.empty()) throw std::runtime_error(name+": "+errors.front());
                if (polygon.vertices.size()>64) throw std::runtime_error(name+": use at most 64 vertices per outline.");
                raw.push_back({std::move(polygon),name,circle});
                if (raw.size()>250) throw std::runtime_error("Import up to 250 boundary rings at a time.");
            };
            for (size_t index=0;index<entities.size();++index)
            {
                const auto& entity=entities[index];
                const std::string layer=field(entity,8,"0");
                const std::string name="DXF "+layer+" "+std::to_string(raw.size()+1);
                if (entity.type=="LWPOLYLINE")
                {
                    requireStraight(entity);
                    if ((integer(field(entity,70,"0"))&1)==0) throw std::runtime_error("An open polyline was found. Close the cutting outline before importing.");
                    NestingGeometry::Polygon polygon;
                    bool needsY=false;
                    for (const auto& group:entity.groups)
                    {
                        if (group.code==10)
                        {
                            if (needsY) throw std::runtime_error("Polyline vertex is missing its Y coordinate.");
                            polygon.vertices.push_back({number(group.value)*scale,0}); needsY=true;
                        }
                        if (group.code==20)
                        {
                            if (!needsY) throw std::runtime_error("Polyline Y coordinate has no X coordinate.");
                            polygon.vertices.back().y=number(group.value)*scale; needsY=false;
                        }
                    }
                    if (needsY || integer(field(entity,90,"-1"))!=static_cast<int>(polygon.vertices.size()))
                        throw std::runtime_error("Polyline vertex count or coordinates are incomplete.");
                    add(std::move(polygon),name);
                }
                else if (entity.type=="POLYLINE")
                {
                    requireStraight(entity);
                    const int flags=integer(field(entity,70,"0"));
                    if ((flags&1)==0 || (flags&126)!=0) throw std::runtime_error("Only closed, straight 2D POLYLINE entities are supported.");
                    NestingGeometry::Polygon polygon;
                    bool ended=false;
                    while (++index<entities.size())
                    {
                        const auto& vertex=entities[index];
                        if (vertex.type=="SEQEND") { ended=true; break; }
                        if (vertex.type!="VERTEX") throw std::runtime_error("POLYLINE is missing SEQEND.");
                        requireStraight(vertex);
                        if ((integer(field(vertex,70,"0"))&~0)!=0) throw std::runtime_error("3D/fitted polyline vertices are unsupported.");
                        polygon.vertices.push_back({required(vertex,10)*scale,required(vertex,20)*scale});
                    }
                    if (!ended) throw std::runtime_error("POLYLINE is missing SEQEND.");
                    add(std::move(polygon),name);
                }
                else if (entity.type=="CIRCLE")
                {
                    require2d(entity);
                    const double radius=required(entity,40)*scale;
                    auto polygon=NestingGeometry::circle(radius*2);
                    const double cx=required(entity,10)*scale,cy=required(entity,20)*scale;
                    for (auto& point:polygon.vertices) { point.x+=cx-radius; point.y+=cy-radius; }
                    add(std::move(polygon),"Circle",true);
                }
                else if (entity.type=="LINE")
                {
                    require2d(entity);
                    Segment segment{{required(entity,10)*scale,required(entity,20)*scale},
                        {required(entity,11)*scale,required(entity,21)*scale},layer};
                    if (pointsNear(segment.a,segment.b)) throw std::runtime_error("DXF contains a zero-length line.");
                    segments.push_back(segment);
                    if (segments.size()>3200) throw std::runtime_error("Too many line segments in this DXF.");
                }
                else if (entity.type=="TEXT" || entity.type=="MTEXT" || entity.type=="DIMENSION" || entity.type=="POINT") skippedAnnotations=true;
                else throw std::runtime_error("Unsupported DXF entity: "+entity.type+". Export closed straight polylines or circles.");
            }
            for (size_t first=0;first<segments.size();++first)
            {
                if (segments[first].used) continue;
                auto& start=segments[first]; start.used=true;
                NestingGeometry::Polygon polygon{{start.a,start.b}};
                while (!pointsNear(polygon.vertices.back(),polygon.vertices.front()))
                {
                    size_t match=segments.size(); bool reverse=false;
                    for (size_t candidate=0;candidate<segments.size();++candidate)
                    {
                        const auto& edge=segments[candidate];
                        if (edge.used || edge.layer!=start.layer) continue;
                        if (pointsNear(edge.a,polygon.vertices.back()) || pointsNear(edge.b,polygon.vertices.back()))
                        {
                            if (match!=segments.size()) throw std::runtime_error("DXF line chains branch or overlap. Export each outline as a closed polyline.");
                            match=candidate; reverse=pointsNear(edge.b,polygon.vertices.back());
                        }
                    }
                    if (match==segments.size()) throw std::runtime_error("An open line chain was found. Close each cutting outline before importing.");
                    segments[match].used=true;
                    polygon.vertices.push_back(reverse ? segments[match].a : segments[match].b);
                    if (polygon.vertices.size()>65) throw std::runtime_error("A line outline exceeds 64 vertices.");
                }
                add(std::move(polygon),"DXF "+start.layer+" "+std::to_string(raw.size()+1));
            }
            if (raw.empty()) throw std::runtime_error("No supported closed cutting outlines were found.");
            // Build a containment forest independent of layer, entity order and
            // winding. Odd depths are cutouts; even depths are separate parts.
            const size_t none=raw.size();
            std::vector<size_t> parent(raw.size(),none),depth(raw.size(),0);
            auto contain=[&](size_t outer,size_t inner)
            {
                if (parent[inner]==none || area(raw[outer].polygon)<area(raw[parent[inner]].polygon)) parent[inner]=outer;
            };
            for (size_t i=0;i<raw.size();++i) for (size_t j=i+1;j<raw.size();++j)
            {
                switch (ringRelation(raw[i].polygon,raw[j].polygon))
                {
                case RingRelation::Contains: contain(i,j); break;
                case RingRelation::Inside: contain(j,i); break;
                case RingRelation::Intersecting:
                    throw std::runtime_error("DXF boundaries cross, touch or duplicate each other. Separate the closed cutting rings.");
                default: break;
                }
            }
            for (size_t i=0;i<raw.size();++i)
            {
                for (size_t ancestor=parent[i];ancestor!=none;ancestor=parent[ancestor]) ++depth[i];
                if (depth[i]%2 && raw[i].circle)
                {
                    // Outer circles enclose the real disk. A circular cutout
                    // must instead be inscribed, never overstating free space.
                    const auto box=bounds(raw[i].polygon);
                    const double cx=(box.left+box.right)/2,cy=(box.top+box.bottom)/2;
                    const double shrink=std::cos(3.14159265358979323846/64);
                    mapVertices(raw[i].polygon,[&](Point p) { return Point{cx+(p.x-cx)*shrink,cy+(p.y-cy)*shrink}; });
                }
            }
            for (size_t i=0;i<raw.size();++i)
                if (parent[i]!=none && ringRelation(raw[parent[i]].polygon,raw[i].polygon)!=RingRelation::Contains)
                    throw std::runtime_error("Circular approximation leaves insufficient separation between nested DXF rings.");
            std::vector<Outline> profiles;
            for (size_t i=0;i<raw.size();++i)
            {
                if (depth[i]%2) continue;
                auto outline=raw[i];
                for (size_t j=0;j<raw.size();++j)
                    if (parent[j]==i) outline.polygon.holes.push_back(raw[j].polygon);
                if (outline.polygon.holes.size()>16) throw std::runtime_error("Use up to 16 holes per profile.");
                const auto errors=validateOutline(outline.polygon);
                if (!errors.empty()) throw std::runtime_error(errors.front());
                const auto box=bounds(outline.polygon);
                // Map CAD Y-up to the screen's Y-down coordinates, retaining size.
                mapVertices(outline.polygon,[&](Point p) { return Point{p.x-box.left,box.bottom-p.y}; });
                profiles.push_back(std::move(outline));
            }
            if (profiles.size()>50) throw std::runtime_error("Import up to 50 profiles at a time.");
            result.outlines=std::move(profiles);
            if (skippedAnnotations) result.warnings.push_back("Text, dimensions and point annotations were skipped.");
        }
        catch (const std::exception& error)
        { result.outlines.clear(); result.error=error.what(); }
        return result;
    }
    Result load(const std::filesystem::path& path,double unitlessToMillimetres)
    {
        try
        {
            std::ifstream file(path,std::ios::binary|std::ios::ate);
            if (!file) throw std::runtime_error("The DXF file could not be opened.");
            const auto length=file.tellg();
            if (length<0 || length>8*1024*1024) throw std::runtime_error("DXF files must be smaller than 8 MB.");
            std::string data(static_cast<size_t>(length),'\0');
            file.seekg(0); if (!file.read(data.data(),static_cast<std::streamsize>(data.size()))) throw std::runtime_error("The DXF file could not be read completely.");
            return parse(data,unitlessToMillimetres);
        }
        catch (const std::exception& error) { Result failed; failed.error=error.what(); return failed; }
    }
    std::filesystem::path chooseFile(std::string& error)
    {
        error.clear();
        std::vector<wchar_t> filename(32768,L'\0');
        OPENFILENAMEW dialog{}; dialog.lStructSize=sizeof(dialog); dialog.hwndOwner=GetActiveWindow();
        dialog.lpstrFilter=L"DXF cutting outlines (*.dxf)\0*.dxf\0";
        dialog.lpstrFile=filename.data(); dialog.nMaxFile=static_cast<DWORD>(filename.size());
        dialog.lpstrTitle=L"Import DXF outlines";
        dialog.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR|OFN_EXPLORER;
        if (GetOpenFileNameW(&dialog)) return std::filesystem::path(filename.data());
        if (CommDlgExtendedError()!=0) error="The DXF file chooser could not be opened.";
        return {};
    }
}
