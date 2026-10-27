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

    void addGroupHeader(const std::string& title);

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
    void setResizableColumns(bool enabled);
    void setHeaderContentGap(int height);
    void setShowColumnHeader(bool enabled);
    void setOuterBorderVisible(bool enabled);

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
        bool groupHeader = false;
    };

    std::vector<Column> columns;
    std::vector<Row> rows;
    int selectedRow = -1;
    int hoveredRow = -1;

    int headerHeight = 36;
    bool showColumnHeader = true;
    bool outerBorderVisible = true;
    int headerContentGap = 0;
    // Keep every grid on the compact sizing used by Quote History.
    int rowHeight = 30;
    int scrollOffset = 0;
    int horizontalOffset = 0;
    bool draggingHorizontalThumb = false;
    int horizontalDragStartX = 0;
    int horizontalDragStartOffset = 0;
    bool resizableColumns = false;
    int resizingColumn = -1;
    int hoveredResizeColumn = -1;
    int resizeStartX = 0;
    int resizeStartLeftWidth = 0;
    LabelStyle textStyle = LabelStyle::Small;
    std::string emptyMessage;

    std::function<void(int)> onSelectionChanged;
};
