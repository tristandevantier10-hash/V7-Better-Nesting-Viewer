#pragma once

#include "UIElement.h"
#include <string>
#include <vector>
#include <functional>

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
        const std::vector<std::string>& values,
        int dataIndex = -1);

    void clear();

    void updateRow(
        int index,
        const std::vector<std::string>& values);

    void removeRow(
        int index);

    int getSelectedRow() const;

    void setSelectionChangedCallback(
        std::function<void(int)> callback);

private:

    struct Column
    {
        std::string title;
        int width;
    };

    struct Row
    {
        std::vector<std::string> values;
        int dataIndex = -1;
    };

    std::vector<Column> columns;
    std::vector<Row> rows;
    int selectedRow = -1;
    int hoveredRow = -1;

    int headerHeight = 36;
    int rowHeight = 52;

    std::function<void(int)> onSelectionChanged;
};