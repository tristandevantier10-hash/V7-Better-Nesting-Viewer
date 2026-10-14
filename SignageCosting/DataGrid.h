#pragma once

#include "UIElement.h"
#include "Label.h"
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

    void setRowHeight(int height);
    void setTextStyle(LabelStyle style);
    void setEmptyMessage(const std::string& message);

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
    // Keep every grid on the compact sizing used by Quote History.
    int rowHeight = 30;
    int scrollOffset = 0;
    LabelStyle textStyle = LabelStyle::Small;
    std::string emptyMessage;

    std::function<void(int)> onSelectionChanged;
};
