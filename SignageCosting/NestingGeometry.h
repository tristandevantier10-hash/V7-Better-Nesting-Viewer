#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace NestingGeometry
{
    using PartId = std::uint64_t;
    PartId newPartId();

    // Coordinates and clearances are in millimetres. Rings are implicitly closed.
    // This foundation accepts simple convex/concave outlines; holes are separate
    // future geometry, and must not be represented by a self-intersecting ring.
    constexpr double tolerance = 1e-7;
    struct Point { double x = 0, y = 0; };
    struct Polygon { std::vector<Point> vertices; };
    struct Bounds { double left = 0, top = 0, right = 0, bottom = 0; };
    struct Transform { double x = 0, y = 0, rotationDegrees = 0; };
    struct Part
    {
        PartId id = 0;
        Polygon outline;
        int quantity = 0;
        std::vector<double> allowedRotations{0, 90};
        std::string name;
    };
    struct Placement
    {
        PartId partId = 0;
        std::uint32_t instanceIndex = 0; // Zero based, stable within a source part.
        Transform transform;
    };
    struct SheetLayout
    {
        double width = 0, height = 0;
        std::vector<Placement> placements;
    };
    enum class Method { Rectangles, Contours };

    std::vector<std::string> validateOutline(const Polygon& polygon);
    Polygon rectangle(double width, double height);
    // A conservative 64-sided circle envelope; its bounds equal the diameter.
    Polygon circle(double diameter);
    Polygon transformed(const Polygon& polygon, const Transform& transform);
    Bounds bounds(const Polygon& polygon);
    double area(const Polygon& polygon);
    bool isRectangle(const Polygon& polygon);
    Method methodFor(const std::vector<Part>& parts);

    // At zero clearance, edge/point contact is permitted; positive-area overlap
    // is a collision. Invalid outlines/clearances throw, never silently pass.
    bool collide(const Polygon& a, const Polygon& b, double clearance = 0);
    bool fitsSheet(const Polygon& polygon, double width, double height, double margin = 0);
    std::vector<std::string> validateLayout(const std::vector<Part>& parts,
        const std::vector<SheetLayout>& sheets, double clearance = 0, double margin = 0);
}
