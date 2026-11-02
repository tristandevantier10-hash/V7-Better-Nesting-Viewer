#include "../DxfImport.h"
#include "../NestingService.h"
#include "../NestingGeometryBridge.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
void check(bool b,const char* s){if(!b)throw std::runtime_error(s);}
std::string document(std::string entities,int units=4){return "0\nSECTION\n2\nHEADER\n9\n$INSUNITS\n70\n"+std::to_string(units)+"\n0\nENDSEC\n0\nSECTION\n2\nENTITIES\n"+entities+"0\nENDSEC\n0\nEOF\n";}
std::string square="0\nLWPOLYLINE\n90\n4\n70\n1\n10\n0\n20\n0\n10\n10\n20\n0\n10\n10\n20\n10\n10\n0\n20\n10\n";
int main(){try{
 auto mm=DxfImport::parse(document(square));check(mm.valid(),"closed lightweight polyline");check(std::abs(NestingGeometry::area(mm.outlines[0].polygon)-100)<1e-6,"mm area");
 auto inches=DxfImport::parse(document(square,1));check(inches.valid()&&std::abs(NestingGeometry::bounds(inches.outlines[0].polygon).right-254)<1e-6,"inch conversion");
 auto unknown=DxfImport::parse(document(square,0),25.4);check(unknown.valid()&&unknown.unitless&&std::abs(NestingGeometry::bounds(unknown.outlines[0].polygon).right-254)<1e-6,"unitless override");
 auto disk=DxfImport::parse(document("0\nCIRCLE\n10\n-20\n20\n30\n40\n5\n"));check(disk.valid()&&disk.outlines[0].circle,"circle import");
 auto c=disk.outlines[0].polygon;check(c.vertices.size()==64&&std::abs(NestingGeometry::bounds(c).right-10)<1e-6,"circle diameter");check(NestingGeometry::area(c)>25*3.14159265&&NestingGeometry::area(c)<25*3.14159265*1.001,"conservative circle area");check(!NestingGeometry::collide(c,NestingGeometry::transformed(c,{10,0,0})),"touching circles");check(NestingGeometry::collide(c,NestingGeometry::transformed(c,{9,0,0})),"overlapping circles");
 auto classic=DxfImport::parse(document("0\nPOLYLINE\n70\n1\n0\nVERTEX\n10\n0\n20\n0\n0\nVERTEX\n10\n10\n20\n0\n0\nVERTEX\n10\n0\n20\n10\n0\nSEQEND\n"));check(classic.valid(),"classic polyline");
 auto lines=DxfImport::parse(document("0\nLINE\n10\n0\n20\n0\n11\n10\n21\n0\n0\nLINE\n10\n0\n20\n10\n11\n10\n21\n0\n0\nLINE\n10\n0\n20\n0\n11\n0\n21\n10\n"));check(lines.valid(),"joined reversed lines");
 auto rejected=[&](std::string input){auto r=DxfImport::parse(input);check(!r.valid()&&!r.error.empty()&&r.outlines.empty(),"invalid import must be atomic");};
 rejected(document(square+square));rejected(document(square+"0\nCIRCLE\n10\n5\n20\n5\n40\n1\n"));rejected(document(square+"0\nARC\n"));rejected(document(square+"42\n1\n"));rejected(document(square+"38\n1\n"));rejected(document("0\nLINE\n10\n0\n20\n0\n11\n10\n21\n0\n"));rejected("0\nSECTION\n");rejected("AutoCAD Binary DXF");
 auto open=square;open.replace(open.find("70\n1"),4,"70\n0");rejected(document(open));
 std::vector<NestingGeometry::Part> parts{{901,NestingGeometry::circle(100),4,{0},"Circle"}};NestingService::Options options{200,200};auto nest=NestingService::calculate(parts,options);check(nest.layout.isValid()&&nest.layout.sheets.size()==1,"four circles one sheet");check(NestingGeometry::validateLayout(parts,NestingGeometry::contourLayouts(nest.layout.sheets)).empty(),"circle nest validation");
 std::cout<<"DXF and circle checks passed.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
