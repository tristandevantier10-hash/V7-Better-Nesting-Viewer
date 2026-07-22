#pragma once
#include <vector>
#include <algorithm>

// Nesting Rectangles defintions //

struct Rect {
    double width = 0;
    double height = 0;
    int quantity = 0;
};

struct PlacedRect
{
    double x = 0;
    double y = 0;

    double width = 0;
    double height = 0;

    bool rotated = false;
};

struct Sheet {

    double width = 0;
    double height = 0;


    // OLD SYSTEM (keep temporarily)
    double cursorX = 0;
    double cursorY = 0;
    double rowHeight = 0;


    // EXISTING RESULTS
    std::vector<PlacedRect> placed;


    // NEW MAXRECTS SYSTEM
    std::vector<PlacedRect> freeRects;
};

struct RollState {
    double width = 0;
    double cursorX = 0;
    double rowHeight = 0;
    double usedLength = 0;
};

class NestingEngine {
public:
    // SHEETS
    double sheetWidth = 2440;
    double sheetHeight = 1220;

    bool canFit(Sheet& sheet, double w, double h);
    void placeRect(
        Sheet& sheet,
        double w,
        double h,
        bool rotated);
    void newRow(Sheet& sheet);
    void newSheet(std::vector<Sheet>& sheets);
    std::vector<Sheet> calculateSheets(
        const std::vector<Rect>& items);

    // VINYL ROLL
    double calculateRollMeters(const std::vector<Rect>& items, double rollWidth);
};