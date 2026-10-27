#include "DataGrid.h"
#include "Renderer.h"
#include "Label.h"
#include "Theme.h"
#include <algorithm>
#include <cstdlib>

namespace
{
    constexpr int categoryListGap = 0;
    constexpr int categoryTopGap = 20;
    constexpr int categoryHeadingGap = 20;
    constexpr int categoryColumnsHeight = 32;
}

DataGrid::DataGrid()
{}

void DataGrid::addColumn(
    const std::string& title,
    int width)
{
    columns.push_back(
        {
            title,
            width
        });
}

void DataGrid::update(const SDL_Event& e)
{
    hoveredRow = -1;

    int mouseX = 0;
    int mouseY = 0;
    if (e.type == SDL_MOUSEMOTION)
    {
        mouseX = e.motion.x;
        mouseY = e.motion.y;
    }
    else if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP)
    {
        mouseX = e.button.x;
        mouseY = e.button.y;
    }
    else
    {
        SDL_GetMouseState(&mouseX, &mouseY);
    }

    const bool mouseInside =
        mouseX >= getX() && mouseX < getX() + getWidth() &&
        mouseY >= getY() && mouseY < getY() + getHeight();

    const int topHeaderHeight = showColumnHeader ? headerHeight : 0;
    const int bodyTop = getY() + topHeaderHeight + headerContentGap;
    const bool mouseInsideBody = mouseInside && mouseY >= bodyTop;
    int contentWidth = 0;
    for (const auto& column : columns)
        contentWidth += column.width;
    const bool hasHorizontalOverflow = contentWidth > getWidth();
    const int horizontalBarHeight = hasHorizontalOverflow ? 9 : 0;
    const int bodyViewportHeight = std::max(0,
        getHeight() - topHeaderHeight - headerContentGap - horizontalBarHeight);
    const int maxHorizontalScroll = std::max(0, contentWidth - getWidth());
    horizontalOffset = std::max(0, std::min(horizontalOffset, maxHorizontalScroll));

    if (draggingHorizontalThumb && e.type == SDL_MOUSEMOTION)
    {
        const int thumbWidth = std::max(24, getWidth() * getWidth() / std::max(getWidth(), contentWidth));
        const int thumbTravel = std::max(1, getWidth() - thumbWidth);
        horizontalOffset = std::max(0, std::min(maxHorizontalScroll,
            horizontalDragStartOffset + (e.motion.x - horizontalDragStartX) * maxHorizontalScroll / thumbTravel));
        return;
    }
    if (draggingHorizontalThumb && e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT)
    {
        draggingHorizontalThumb = false;
        return;
    }

    if (resizingColumn >= 0 && e.type == SDL_MOUSEMOTION)
    {
        const int delta = e.motion.x - resizeStartX;
        const int minimumWidth = 56;
        const int maximumWidth = std::max(600, getWidth() * 2);
        columns[resizingColumn].width = std::max(minimumWidth,
            std::min(resizeStartLeftWidth + delta, maximumWidth));
        return;
    }

    if (resizingColumn >= 0 && e.type == SDL_MOUSEBUTTONUP &&
        e.button.button == SDL_BUTTON_LEFT)
    {
        resizingColumn = -1;
        return;
    }

    hoveredResizeColumn = -1;
    if (resizableColumns && mouseInside)
    {
        bool inColumnHeader = showColumnHeader && mouseY >= getY() && mouseY < getY() + headerHeight;
        int rowY = bodyTop - scrollOffset;
        for (const auto& row : rows)
        {
            if (row.groupHeader)
            {
                if (mouseY >= rowY + categoryTopGap + rowHeight + categoryHeadingGap &&
                    mouseY < rowY + categoryTopGap + rowHeight + categoryHeadingGap + categoryColumnsHeight)
                    inColumnHeader = true;
            }
            else if (mouseY >= rowY && mouseY < rowY + rowHeight)
            {
                inColumnHeader = true;
            }
            rowY += rowHeight + (row.groupHeader ? categoryTopGap + categoryHeadingGap + categoryColumnsHeight + categoryListGap : 0);
        }
        if (inColumnHeader)
        {
            int boundaryX = getX() - horizontalOffset;
            for (std::size_t i = 0; i + 1 < columns.size(); ++i)
            {
                boundaryX += columns[i].width;
                if (std::abs(mouseX - boundaryX) <= 5)
                {
                    hoveredResizeColumn = static_cast<int>(i);
                    break;
                }
            }
        }
    }

    if (resizableColumns && e.type == SDL_MOUSEBUTTONDOWN &&
        e.button.button == SDL_BUTTON_LEFT && hoveredResizeColumn >= 0)
    {
        resizingColumn = hoveredResizeColumn;
        resizeStartX = e.button.x;
        resizeStartLeftWidth = columns[resizingColumn].width;
        return;
    }

    const int horizontalTrackY = getY() + getHeight() - horizontalBarHeight;
    if (hasHorizontalOverflow && e.type == SDL_MOUSEBUTTONDOWN &&
        e.button.button == SDL_BUTTON_LEFT && e.button.y >= horizontalTrackY &&
        e.button.y < getY() + getHeight())
    {
        const int thumbWidth = std::max(24, getWidth() * getWidth() / contentWidth);
        const int thumbTravel = std::max(1, getWidth() - thumbWidth);
        const int thumbX = getX() + (maxHorizontalScroll > 0 ? horizontalOffset * thumbTravel / maxHorizontalScroll : 0);
        if (e.button.x >= thumbX && e.button.x < thumbX + thumbWidth)
        {
            draggingHorizontalThumb = true;
            horizontalDragStartX = e.button.x;
            horizontalDragStartOffset = horizontalOffset;
        }
        else
        {
            horizontalOffset = std::max(0, std::min(maxHorizontalScroll,
                (e.button.x - getX() - thumbWidth / 2) * maxHorizontalScroll / thumbTravel));
        }
        return;
    }

    if (e.type == SDL_MOUSEWHEEL && mouseInsideBody)
    {
        int contentHeight = 0;
        for (const auto& row : rows)
            contentHeight += rowHeight + (row.groupHeader ? categoryTopGap + categoryHeadingGap + categoryColumnsHeight + categoryListGap : 0);
        const int viewportHeight = bodyViewportHeight;
        const int maxScroll = std::max(0, contentHeight - viewportHeight);
        scrollOffset -= e.wheel.y * rowHeight * 3;
        scrollOffset = std::max(0, std::min(scrollOffset, maxScroll));
        if (e.wheel.x != 0)
            horizontalOffset = std::max(0, std::min(maxHorizontalScroll,
                horizontalOffset + e.wheel.x * 60));
    }

    if (mouseInsideBody)
    {
        const int rowsTop = bodyTop;
        if (mouseY >= rowsTop)
        {
            const int rowOffset = mouseY - rowsTop + scrollOffset;
            int consumed = 0;
            for (std::size_t i = 0; i < rows.size(); ++i)
            {
                const int height = rowHeight + (rows[i].groupHeader ? categoryTopGap + categoryHeadingGap + categoryColumnsHeight + categoryListGap : 0);
                if (rowOffset >= consumed && rowOffset < consumed + height)
                {
                    hoveredRow = rows[i].groupHeader || rowOffset >= consumed + rowHeight
                        ? -1 : static_cast<int>(i);
                    break;
                }
                consumed += height;
            }
        }
    }

    if (e.type == SDL_MOUSEBUTTONDOWN &&
        e.button.button == SDL_BUTTON_LEFT)
    {
        if (e.button.x >= getX() && e.button.x < getX() + getWidth() &&
            e.button.y >= bodyTop && e.button.y < getY() + getHeight())
        {
            const int rowOffset = e.button.y - bodyTop + scrollOffset;
            int consumed = 0;
            int clickedRow = -1;
            for (std::size_t i = 0; i < rows.size(); ++i)
            {
                const int height = rowHeight + (rows[i].groupHeader ? categoryTopGap + categoryHeadingGap + categoryColumnsHeight + categoryListGap : 0);
                if (rowOffset >= consumed && rowOffset < consumed + rowHeight && !rows[i].groupHeader)
                {
                    clickedRow = static_cast<int>(i);
                    break;
                }
                consumed += height;
            }
            if (clickedRow >= 0)
            {
                selectedRow = clickedRow;
                if (onSelectionChanged)
                    onSelectionChanged(selectedRow);
            }
        }
    }
}

void DataGrid::render(Renderer& renderer)
{

    if (!visible)
        return;

    renderer.pushClip(bounds);

    //--------------------------------------------------
    // Background
    //--------------------------------------------------

    SDL_Rect background =
    {
        getX(),
        getY(),
        getWidth(),
        getHeight()
    };

    renderer.fillRect(
        background,
        DefaultTheme.panelBackground);

    //--------------------------------------------------
    // Header
    //--------------------------------------------------

    if (showColumnHeader)
    {
        SDL_Rect header = { getX(), getY(), getWidth(), headerHeight };
        renderer.fillRect(header, DefaultTheme.mutedSurface);

    //--------------------------------------------------
    // Column titles
    //--------------------------------------------------

    int x = getX() - horizontalOffset;

    for (const auto& column : columns)
    {
        SDL_Rect headerClip =
        {
            x,
            getY(),
            column.width,
            headerHeight
        };

        renderer.pushClip(headerClip);

        renderer.drawText(
            column.title,
            x + 10,
            getY() + (headerHeight - 16) / 2,
            LabelStyle::Normal,
            DefaultTheme.darkText);

        renderer.popClip();

        x += column.width;

        renderer.drawLine(
            x,
            getY(),
            x,
            getY() + headerHeight,
            DefaultTheme.border);
    }

    //--------------------------------------------------
    // Bottom header line
    //--------------------------------------------------

    renderer.drawLine(
        getX(),
        getY() + headerHeight,
        getX() + getWidth(),
        getY() + headerHeight,
        DefaultTheme.border);

    int resizeBoundaryX = getX() - horizontalOffset;
    for (std::size_t i = 0; i + 1 < columns.size(); ++i)
    {
        resizeBoundaryX += columns[i].width;
        if (static_cast<int>(i) != hoveredResizeColumn &&
            static_cast<int>(i) != resizingColumn)
            continue;

        renderer.fillRect(
            { resizeBoundaryX - 1, getY(), 3, headerHeight },
            DefaultTheme.accent);
    }
    }

    //--------------------------------------------------
    // Rows
    //--------------------------------------------------

    int contentWidth = 0;
    for (const auto& column : columns)
        contentWidth += column.width;
    const bool hasHorizontalOverflow = contentWidth > getWidth();
    const int horizontalBarHeight = hasHorizontalOverflow ? 9 : 0;
    const int topHeaderHeight = showColumnHeader ? headerHeight : 0;
    const int rowViewportHeight = std::max(0, getHeight() - topHeaderHeight - headerContentGap - horizontalBarHeight);
    const int rowsTop = getY() + topHeaderHeight + headerContentGap;
    if (headerContentGap > 0)
    {
        renderer.fillRect(
            { getX(), getY() + topHeaderHeight, getWidth(), headerContentGap },
            DefaultTheme.panelBackground);
    }
    renderer.pushClip({ getX(), rowsTop, getWidth(), rowViewportHeight });
    if (rows.empty() && !emptyMessage.empty() && rowViewportHeight > 0)
    {
        const int textWidth = renderer.getTextWidth(emptyMessage);
        renderer.drawText(
            emptyMessage,
            getX() + (getWidth() - textWidth) / 2,
            rowsTop + (rowViewportHeight - 16) / 2,
            LabelStyle::Small,
            DefaultTheme.darkSecondaryText);
    }

    int consumedHeight = 0;
    std::size_t firstVisibleRow = 0;
    while (firstVisibleRow < rows.size() && consumedHeight + rowHeight +
        (rows[firstVisibleRow].groupHeader ? categoryTopGap + categoryHeadingGap + categoryColumnsHeight + categoryListGap : 0) <= scrollOffset)
    {
        consumedHeight += rowHeight + (rows[firstVisibleRow].groupHeader ? categoryTopGap + categoryHeadingGap + categoryColumnsHeight + categoryListGap : 0);
        ++firstVisibleRow;
    }
    int y = rowsTop + consumedHeight - scrollOffset;
    int x = getX() - horizontalOffset;

    std::vector<SDL_Rect> categoryTableBorders;
    int sectionY = rowsTop - scrollOffset;
    for (std::size_t i = 0; i < rows.size(); ++i)
    {
        if (rows[i].groupHeader)
        {
            std::size_t sectionRows = 0;
            for (std::size_t j = i + 1; j < rows.size() && !rows[j].groupHeader; ++j)
                ++sectionRows;
            const int tableTop = sectionY + categoryTopGap + rowHeight + categoryHeadingGap;
            const int tableHeight = categoryColumnsHeight + categoryListGap +
                static_cast<int>(sectionRows) * rowHeight;
            categoryTableBorders.push_back({ getX(), tableTop, getWidth(), tableHeight });
            sectionY += rowHeight + categoryTopGap + categoryHeadingGap + categoryColumnsHeight + categoryListGap;
        }
        else
        {
            sectionY += rowHeight;
        }
    }

    for (size_t rowIndex = firstVisibleRow; rowIndex < rows.size() && y < rowsTop + rowViewportHeight; ++rowIndex)
    {

        SDL_Color rowColour;

        if (rows[rowIndex].groupHeader)
        {
            y += categoryTopGap;
            renderer.fillRect(
                { getX(), y, getWidth(), rowHeight },
                DefaultTheme.successSurface);
            renderer.fillRect(
                { getX(), y, 3, rowHeight },
                DefaultTheme.accent);
            renderer.drawText(
                rows[rowIndex].values.empty() ? std::string{} : rows[rowIndex].values.front(),
                getX() + 12,
                y + (rowHeight - 16) / 2,
                LabelStyle::Normal,
                DefaultTheme.darkText);
            renderer.drawLine(
                getX(), y + rowHeight - 1, getX() + getWidth(), y + rowHeight - 1,
                DefaultTheme.border);
            const int columnHeaderY = y + rowHeight + categoryHeadingGap;
            renderer.fillRect({ getX(), columnHeaderY, getWidth(), categoryColumnsHeight },
                DefaultTheme.mutedSurface);
            int columnX = getX() - horizontalOffset;
            for (std::size_t col = 0; col < columns.size(); ++col)
            {
                renderer.pushClip({ columnX, columnHeaderY, columns[col].width, categoryColumnsHeight });
                renderer.drawText(columns[col].title, columnX + 10,
                    columnHeaderY + (categoryColumnsHeight - 16) / 2,
                    LabelStyle::Normal, DefaultTheme.darkText);
                renderer.popClip();
                columnX += columns[col].width;
                renderer.drawLine(columnX, columnHeaderY, columnX,
                    columnHeaderY + categoryColumnsHeight, DefaultTheme.border);
                if (static_cast<int>(col) == hoveredResizeColumn || static_cast<int>(col) == resizingColumn)
                    renderer.fillRect({ columnX - 1, columnHeaderY, 3, categoryColumnsHeight }, DefaultTheme.accent);
            }
            renderer.drawLine(getX(), columnHeaderY + categoryColumnsHeight - 1,
                getX() + getWidth(), columnHeaderY + categoryColumnsHeight - 1, DefaultTheme.border);
            y += rowHeight + categoryHeadingGap + categoryColumnsHeight + categoryListGap;
            continue;
        }

        if ((int)rowIndex == selectedRow)
        {
            rowColour = DefaultTheme.successSurface;
        }
        else if ((int)rowIndex == hoveredRow)
        {
            rowColour = DefaultTheme.windowBackground;
        }
        else
        {
            rowColour =
                (rowIndex % 2 == 0)
                ? DefaultTheme.panelBackground
                : DefaultTheme.mutedSurface;
        }

        SDL_Rect rowRect =
        {
            getX(),
            y,
            getWidth(),
            rowHeight
        };

        renderer.fillRect(
            rowRect,
            rowColour);

        x = getX() - horizontalOffset;

        for (size_t col = 0;
            col < rows[rowIndex].values.size() &&
            col < columns.size();
            ++col)
        {
            SDL_Color textColour = DefaultTheme.darkText;

            SDL_Rect cellClip =
            {
                x,
                y,
                columns[col].width,
                rowHeight
            };

            renderer.pushClip(cellClip);

            renderer.drawText(
                rows[rowIndex].values[col],
                x + 10,
                y + (rowHeight - 16) / 2,
                textStyle,
                textColour);

            renderer.popClip();

            // Draw the separator at the right edge of this column
            const SDL_Color dividerColour =
                static_cast<int>(col) == hoveredResizeColumn || static_cast<int>(col) == resizingColumn
                ? DefaultTheme.accent
                : DefaultTheme.border;
            renderer.drawLine(
                x + columns[col].width,
                y,
                x + columns[col].width,
                y + rowHeight,
                dividerColour);

            x += columns[col].width;
        }

        y += rowHeight;
    }

    for (const SDL_Rect& tableBorder : categoryTableBorders)
        renderer.drawRect(tableBorder, DefaultTheme.border);

    int contentHeight = 0;
    for (const auto& row : rows)
        contentHeight += rowHeight + (row.groupHeader ? categoryTopGap + categoryHeadingGap + categoryColumnsHeight + categoryListGap : 0);
    const int viewportHeight = rowViewportHeight;
    if (contentHeight > viewportHeight && viewportHeight > 0)
    {
        const int trackX = getX() + getWidth() - 7;
        renderer.fillRect(
        { trackX, rowsTop, 5, viewportHeight },
            DefaultTheme.mutedSurface);

        const int thumbHeight = std::max(20,
            viewportHeight * viewportHeight / contentHeight);
        const int maxScroll = contentHeight - viewportHeight;
        const int thumbTravel = viewportHeight - thumbHeight;
        const int thumbY = rowsTop +
            (maxScroll > 0 ? scrollOffset * thumbTravel / maxScroll : 0);
        renderer.fillRect(
            { trackX, thumbY, 5, thumbHeight },
            DefaultTheme.darkSecondaryText);
    }
    renderer.popClip();

    if (hasHorizontalOverflow && getWidth() > 0)
    {
        const int trackY = getY() + getHeight() - horizontalBarHeight;
        renderer.fillRect({ getX(), trackY, getWidth(), horizontalBarHeight },
            DefaultTheme.mutedSurface);
        const int thumbWidth = std::max(24, getWidth() * getWidth() / contentWidth);
        const int thumbTravel = std::max(0, getWidth() - thumbWidth);
        const int maxScroll = contentWidth - getWidth();
        const int thumbX = getX() + (maxScroll > 0 ? horizontalOffset * thumbTravel / maxScroll : 0);
        renderer.fillRect({ thumbX, trackY + 2, thumbWidth, 5 },
            DefaultTheme.darkSecondaryText);
    }

    //--------------------------------------------------
    // Outer Border
    //--------------------------------------------------

    if (outerBorderVisible)
    {
        renderer.drawRect(
            {
                getX(),
                getY(),
                getWidth(),
                getHeight()
            },
            DefaultTheme.border);
    }

    renderer.popClip();
}

void DataGrid::addRow(
    const std::vector<std::string>& values,
    int dataIndex)
{
    rows.push_back({ values, dataIndex, false });
}

void DataGrid::addGroupHeader(const std::string& title)
{
    rows.push_back({ { title }, -1, true });
}

void DataGrid::clear()
{
    rows.clear();
    selectedRow = -1;
    hoveredRow = -1;
    scrollOffset = 0;
}

void DataGrid::updateRow(
    int index,
    const std::vector<std::string>& values)
{
    if (index < 0 ||
        index >= static_cast<int>(rows.size()))
    {
        return;
    }

    rows[index].values = values;
}

void DataGrid::removeRow(int index)
{
    if (index < 0 ||
        index >= static_cast<int>(rows.size()))
    {
        return;
    }

    rows.erase(
        rows.begin() + index);

    if (selectedRow >=
        static_cast<int>(rows.size()))
    {
        selectedRow =
            static_cast<int>(rows.size()) - 1;
    }
}

int DataGrid::getSelectedRow() const
{
    if (selectedRow < 0 ||
        selectedRow >= static_cast<int>(rows.size()) ||
        rows[selectedRow].groupHeader)
    {
        return -1;
    }

    return selectedRow;
}

void DataGrid::setSelectionChangedCallback(
    std::function<void(int)> callback)
{
    onSelectionChanged = callback;
}

void DataGrid::setRowHeight(int height)
{
    rowHeight = std::max(18, height);
    const int viewportHeight = std::max(0, getHeight() - (showColumnHeader ? headerHeight : 0) - headerContentGap);
    int contentHeight = 0;
    for (const auto& row : rows)
        contentHeight += rowHeight + (row.groupHeader ? categoryTopGap + categoryHeadingGap + categoryColumnsHeight + categoryListGap : 0);
    const int maxScroll = std::max(0, contentHeight - viewportHeight);
    scrollOffset = std::min(scrollOffset, maxScroll);
}

void DataGrid::setTextStyle(LabelStyle style)
{
    textStyle = style;
}

void DataGrid::setEmptyMessage(const std::string& message)
{
    emptyMessage = message;
}

void DataGrid::setResizableColumns(bool enabled)
{
    resizableColumns = enabled;
}

void DataGrid::setHeaderContentGap(int height)
{
    headerContentGap = std::max(0, height);
    const int viewportHeight = std::max(0, getHeight() - (showColumnHeader ? headerHeight : 0) - headerContentGap);
    int contentHeight = 0;
    for (const auto& row : rows)
        contentHeight += rowHeight + (row.groupHeader ? categoryTopGap + categoryHeadingGap + categoryColumnsHeight + categoryListGap : 0);
    const int maxScroll = std::max(0, contentHeight - viewportHeight);
    scrollOffset = std::min(scrollOffset, maxScroll);
}

void DataGrid::setShowColumnHeader(bool enabled)
{
    showColumnHeader = enabled;
}

void DataGrid::setOuterBorderVisible(bool enabled)
{
    outerBorderVisible = enabled;
}
