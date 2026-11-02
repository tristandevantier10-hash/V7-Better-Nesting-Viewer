#include "../DxfImport.h"
#include "../NestingService.h"
#include "../NestingGeometryBridge.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <chrono>
void check(bool b,const char* s){if(!b)throw std::runtime_error(s);}
std::string document(std::string entities,int units=4){return "0\nSECTION\n2\nHEADER\n9\n$INSUNITS\n70\n"+std::to_string(units)+"\n0\nENDSEC\n0\nSECTION\n2\nENTITIES\n"+entities+"0\nENDSEC\n0\nEOF\n";}
std::string square="0\nLWPOLYLINE\n90\n4\n70\n1\n10\n0\n20\n0\n10\n10\n20\n0\n10\n10\n20\n10\n10\n0\n20\n10\n";
std::string poly(NestingGeometry::Polygon polygon,const std::string& layer="0")
{
 std::ostringstream out;out<<"0\nLWPOLYLINE\n8\n"<<layer<<"\n90\n"<<polygon.vertices.size()<<"\n70\n1\n";
 for(const auto& p:polygon.vertices)out<<"10\n"<<p.x<<"\n20\n"<<p.y<<"\n";
 return out.str();
}
int main(){try{
 auto mm=DxfImport::parse(document(square));check(mm.valid(),"closed lightweight polyline");check(std::abs(NestingGeometry::area(mm.outlines[0].polygon)-100)<1e-6,"mm area");
 auto inches=DxfImport::parse(document(square,1));check(inches.valid()&&std::abs(NestingGeometry::bounds(inches.outlines[0].polygon).right-254)<1e-6,"inch conversion");
 auto unknown=DxfImport::parse(document(square,0),25.4);check(unknown.valid()&&unknown.unitless&&std::abs(NestingGeometry::bounds(unknown.outlines[0].polygon).right-254)<1e-6,"unitless override");
 auto disk=DxfImport::parse(document("0\nCIRCLE\n10\n-20\n20\n30\n40\n5\n"));check(disk.valid()&&disk.outlines[0].circle,"circle import");
 auto c=disk.outlines[0].polygon;check(c.vertices.size()==64&&std::abs(NestingGeometry::bounds(c).right-10)<1e-6,"circle diameter");check(NestingGeometry::area(c)>25*3.14159265&&NestingGeometry::area(c)<25*3.14159265*1.001,"conservative circle area");check(!NestingGeometry::collide(c,NestingGeometry::transformed(c,{10,0,0})),"touching circles");check(NestingGeometry::collide(c,NestingGeometry::transformed(c,{9,0,0})),"overlapping circles");
 auto classic=DxfImport::parse(document("0\nPOLYLINE\n70\n1\n0\nVERTEX\n10\n0\n20\n0\n0\nVERTEX\n10\n10\n20\n0\n0\nVERTEX\n10\n0\n20\n10\n0\nSEQEND\n"));check(classic.valid(),"classic polyline");
 auto lines=DxfImport::parse(document("0\nLINE\n10\n0\n20\n0\n11\n10\n21\n0\n0\nLINE\n10\n0\n20\n10\n11\n10\n21\n0\n0\nLINE\n10\n0\n20\n0\n11\n0\n21\n10\n"));check(lines.valid(),"joined reversed lines");
 auto rejected=[&](std::string input){auto r=DxfImport::parse(input);check(!r.valid()&&!r.error.empty()&&r.outlines.empty(),"invalid import must be atomic");};
 rejected(document(square+square));rejected(document(square+"0\nARC\n"));rejected(document(square+"42\n1\n"));rejected(document(square+"38\n1\n"));rejected(document("0\nLINE\n10\n0\n20\n0\n11\n10\n21\n0\n"));rejected("0\nSECTION\n");rejected("AutoCAD Binary DXF");
 auto open=square;open.replace(open.find("70\n1"),4,"70\n0");rejected(document(open));
 using namespace NestingGeometry;
 auto outer=transformed(rectangle(10,10),{-30,40,0});
 auto inner=transformed(rectangle(6,4),{-28,41,0});
 std::reverse(inner.vertices.begin(),inner.vertices.end());
 auto frame=DxfImport::parse(document(poly(inner,"CUTOUT")+poly(outer,"OUTER")));
 check(frame.valid()&&frame.outlines.size()==1&&frame.outlines[0].polygon.holes.size()==1,"inner ring becomes a hole, independent of layer/order/winding");
 const auto& profile=frame.outlines[0].polygon;
 check(std::abs(area(profile)-76)<1e-6,"net imported material area");
 auto hb=bounds(profile.holes[0]);check(std::abs(hb.left-2)<1e-6&&std::abs(hb.top-5)<1e-6&&std::abs(hb.bottom-9)<1e-6,"cutout shares parent origin and CAD Y reflection");
 auto inchFrame=DxfImport::parse(document(poly(outer)+poly(inner),1));
 check(inchFrame.valid()&&std::abs(area(inchFrame.outlines[0].polygon)-76*25.4*25.4)<1e-6,"drawing units apply to holes");
 auto roundHole=DxfImport::parse(document(square+"0\nCIRCLE\n10\n5\n20\n5\n40\n1\n"));
 check(roundHole.valid()&&roundHole.outlines.size()==1&&roundHole.outlines[0].polygon.holes.size()==1,"circle becomes a cutout");
 const auto& circleHole=roundHole.outlines[0].polygon.holes[0];
 check(area(circleHole)<3.141592653589793&&area(circleHole)>3.13,"circular hole is inscribed, never overstating usable space");
 for(const auto& p:circleHole.vertices)check(std::hypot(p.x-5,p.y-5)<=1+1e-9,"hole vertices remain inside actual circle");
 std::vector<Part> circularParts{{811,roundHole.outlines[0].polygon,1,{0},"Round cutout"},{812,circle(1),1,{0},"Disk"}};
 auto circularNest=NestingService::calculate(circularParts,{10,10});
 check(circularNest.layout.isValid()&&circularNest.layout.sheets.size()==1,"disk nests inside an imported circular cutout");
 check(validateLayout(circularParts,contourLayouts(circularNest.layout.sheets)).empty(),"circular cutout nest validates");
 auto island=transformed(rectangle(1,1),{-26,42,0});
 auto nested=DxfImport::parse(document(poly(island)+poly(outer)+poly(inner)));
 check(nested.valid()&&nested.outlines.size()==2,"even-depth island is a separate solid part");
 auto two=DxfImport::parse(document(poly(rectangle(30,20))+poly(transformed(rectangle(5,5),{2,2,0}))+poly(transformed(rectangle(5,5),{20,10,0}))));
 check(two.valid()&&two.outlines.size()==1&&two.outlines[0].polygon.holes.size()==2,"multiple cutouts in one quantity row");
 rejected(document(square+poly(transformed(rectangle(4,4),{0,2,0}))));
 rejected(document(square+poly(transformed(rectangle(4,4),{8,2,0}))));
 rejected(document(square+poly(transformed(rectangle(4,4),{2,2,0}))+poly(transformed(rectangle(4,4),{4,3,0}))));
 std::vector<Part> frameParts{{801,profile,1,{0,90},"Imported frame"},{802,rectangle(6,4),1,{0,90},"Insert"}};
 auto importedNest=NestingService::calculate(frameParts,{10,10});
 check(importedNest.layout.isValid()&&importedNest.layout.sheets.size()==1,"import-to-nest cutout workflow");
 check(validateLayout(frameParts,contourLayouts(importedNest.layout.sheets)).empty(),"imported layout independently validates");
 // Reproduce the supplied samples and default sheet shown in the slow-job report.
 auto sampleCircle=DxfImport::load("DXF/02-circular-frame-with-disk.dxf");
 auto sampleRectangle=DxfImport::load("DXF/01-rectangular-frame-with-two-cutouts.dxf");
 // The suite runner uses the application directory; direct runs use repo root.
 if(!sampleCircle.valid())sampleCircle=DxfImport::load("../DXF/02-circular-frame-with-disk.dxf");
 if(!sampleRectangle.valid())sampleRectangle=DxfImport::load("../DXF/01-rectangular-frame-with-two-cutouts.dxf");
 check(sampleCircle.valid()&&sampleRectangle.valid(),"load checked-in circular and rectangular samples");
 for(bool mixed:{false,true}) {
  std::vector<Part> job;
  if(mixed)job.push_back({1001,sampleRectangle.outlines[0].polygon,1,{0,90,180,270},"Rectangular frame"});
  for(const auto& sample:sampleCircle.outlines)job.push_back({newPartId()+2000,sample.polygon,1,{0,90,180,270},sample.name});
  auto start=std::chrono::steady_clock::now();
  auto result=NestingService::calculate(job,{2440,1220});
  const double seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
  check(result.layout.isValid()&&result.layout.sheets.size()==1,"circular sample and mixed screenshot job complete on one sheet");
  check(validateLayout(job,contourLayouts(result.layout.sheets)).empty(),"performance regression layouts retain collision correctness");
  // Generous budget for Debug and slower CI machines; old repeated decomposition
  // takes tens of seconds in Release and substantially longer in Debug.
  check(seconds<10,"circular cutout nesting exceeded the 10-second regression budget");
  std::cout<<(mixed?"Mixed sample":"Circular sample")<<" completed in "<<seconds<<" seconds.\n";
 }
 std::vector<NestingGeometry::Part> parts{{901,NestingGeometry::circle(100),4,{0},"Circle"}};NestingService::Options options{200,200};auto nest=NestingService::calculate(parts,options);check(nest.layout.isValid()&&nest.layout.sheets.size()==1,"four circles one sheet");check(NestingGeometry::validateLayout(parts,NestingGeometry::contourLayouts(nest.layout.sheets)).empty(),"circle nest validation");
 std::cout<<"DXF and circle checks passed.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
