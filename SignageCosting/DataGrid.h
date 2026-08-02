#pragma once

#include "UIElement.h"

#include <string>
#include <vector>

class Renderer;

class DataGrid : public UIElement
{
public:

    DataGrid();

    void addColumn(
        const std::string& title,
        int width);

    void update(const SDL_Event& e) override;

    void render(Renderer& renderer) override;

    void addRow(
        const std::vector<std::string>& values);

    void clear();

private:

    struct Column
    {
        std::string title;
        int width;
    };

    struct Row
    {
        std::vector<std::string> values;
    };

    std::vector<Column> columns;
    std::vector<Row> rows;
    int selectedRow = -1;
    int hoveredRow = -1;

    int headerHeight = 36;
    int rowHeight = 52;
};
