#include "NestingEngine.h"
#include <algorithm>
#include <iostream>

// This is the Nesting Code //

bool NestingEngine::canFit(Sheet& sheet, double w, double h)
{
    return sheet.cursorX + w <= sheet.width;
}

void NestingEngine::placeRect(
    Sheet& sheet,
    double w,
    double h,
    bool rotated)
{
    PlacedRect rect;

    rect.x = sheet.cursorX;
    rect.y = sheet.cursorY;

    rect.width = w;
    rect.height = h;

    rect.rotated = rotated;

    sheet.placed.push_back(rect);

    sheet.cursorX += w;

    sheet.rowHeight = std::max(sheet.rowHeight, h);
}

void NestingEngine::newRow(Sheet& sheet)
{
    sheet.cursorX = 0;
    sheet.cursorY += sheet.rowHeight;
    sheet.rowHeight = 0;
}

void NestingEngine::newSheet(std::vector<Sheet>& sheets)
{
    Sheet s;

    s.width = sheetWidth;
    s.height = sheetHeight;

    s.cursorX = 0;
    s.cursorY = 0;
    s.rowHeight = 0;

    // ----------------------------
    // MAXRECTS START STATE
    // ----------------------------
    PlacedRect root;

    root.x = 0;
    root.y = 0;
    root.width = sheetWidth;
    root.height = sheetHeight;
    root.rotated = false;

    s.freeRects.clear();
    s.freeRects.push_back(root);

    sheets.push_back(s);
}

std::vector<Sheet> NestingEngine::calculateSheets(const std::vector<Rect>& items)
{
    std::cout << "\n===== NESTING ENGINE INPUT =====\n";
    std::cout << "Sheet Width  : " << sheetWidth << "\n";
    std::cout << "Sheet Height : " << sheetHeight << "\n";
    std::cout << "Items Count  : " << items.size() << "\n";

    for (const auto& item : items)
    {
        std::cout << "Item: "
            << item.width << " x "
            << item.height
            << " Qty: "
            << item.quantity
            << "\n";
    }

    std::cout << "================================\n";


    std::vector<Sheet> sheets;

    newSheet(sheets);
    Sheet* sheet = &sheets.back();

    for (const auto& item : items)
    {
        for (int i = 0; i < item.quantity; i++)
        {
            double w = item.width;
            double h = item.height;

            bool rotated = false;

            if (!canFit(*sheet, w, h) && canFit(*sheet, h, w))
            {
                std::swap(w, h);
                rotated = true;
            }

            bool placed = false;

            while (!placed)
            {
                // if no horizontal space - new row
                if (sheet->cursorX + w > sheet->width)
                {
                    newRow(*sheet);
                }

                // NOW check if row overflow AFTER row change
                if (sheet->cursorY + sheet->rowHeight > sheet->height)
                {
                    newSheet(sheets);
                    sheet = &sheets.back();
                    continue;
                }

                // final placement check
                if (sheet->cursorX + w <= sheet->width)
                {
                    placeRect(*sheet, w, h, rotated);
                    placed = true;
                }
                else
                {
                    newRow(*sheet);
                }
            }
        }
    }



    return sheets;
}