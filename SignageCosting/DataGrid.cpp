#include "DataGrid.h"
#include "Renderer.h"
#include "Label.h"
#include "Theme.h"
#include <algorithm>

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
    else
    {
        SDL_GetMouseState(&mouseX, &mouseY);
    }

    const bool mouseInside =
        mouseX >= getX() && mouseX < getX() + getWidth() &&
        mouseY >= getY() && mouseY < getY() + getHeight();

    if (e.type == SDL_MOUSEWHEEL && mouseInside)
    {
        const int contentHeight = static_cast<int>(rows.size()) * rowHeight;
        const int viewportHeight = std::max(0, getHeight() - headerHeight);
        const int maxScroll = std::max(0, contentHeight - viewportHeight);
        scrollOffset -= e.wheel.y * rowHeight * 3;
        scrollOffset = std::max(0, std::min(scrollOffset, maxScroll));
    }

    if (mouseInside)
    {
        const int rowsTop = getY() + headerHeight;
        if (mouseY >= rowsTop)
        {
            hoveredRow =
                (mouseY - rowsTop + scrollOffset) / rowHeight;

            if (hoveredRow >= static_cast<int>(rows.size()))
                hoveredRow = -1;
        }
    }

    if (e.type == SDL_MOUSEBUTTONDOWN &&
        e.button.button == SDL_BUTTON_LEFT)
    {
        if (e.button.x >= getX() && e.button.x < getX() + getWidth() &&
            e.button.y >= getY() + headerHeight && e.button.y < getY() + getHeight())
        {
            const int clickedRow =
                (e.button.y - (getY() + headerHeight) + scrollOffset) / rowHeight;
            if (clickedRow >= 0 && clickedRow < static_cast<int>(rows.size()))
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

    SDL_Rect header =
    {
        getX(),
        getY(),
        getWidth(),
        headerHeight
    };

    renderer.fillRect(
        header,
        DefaultTheme.mutedSurface);

    //--------------------------------------------------
    // Column titles
    //--------------------------------------------------

    int x = getX();

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

    //--------------------------------------------------
    // Rows
    //--------------------------------------------------

    const size_t firstVisibleRow =
        static_cast<size_t>(scrollOffset / rowHeight);
    const int rowViewportHeight = std::max(0, getHeight() - headerHeight);
    if (rows.empty() && !emptyMessage.empty() && rowViewportHeight > 0)
    {
        const int textWidth = renderer.getTextWidth(emptyMessage);
        renderer.drawText(
            emptyMessage,
            getX() + (getWidth() - textWidth) / 2,
            getY() + headerHeight + (rowViewportHeight - 16) / 2,
            LabelStyle::Small,
            DefaultTheme.darkSecondaryText);
    }

    const size_t visibleRowCount =
        static_cast<size_t>((rowViewportHeight + rowHeight - 1) / rowHeight + 1);
    const size_t lastVisibleRow =
        std::min(rows.size(), firstVisibleRow + visibleRowCount);
    int y = getY() + headerHeight - (scrollOffset % rowHeight);

    for (size_t rowIndex = firstVisibleRow;
        rowIndex < lastVisibleRow;
        ++rowIndex)
    {

        SDL_Color rowColour;

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

        x = getX();

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
            renderer.drawLine(
                x + columns[col].width,
                y,
                x + columns[col].width,
                y + rowHeight,
                DefaultTheme.border);

            x += columns[col].width;
        }

        y += rowHeight;
    }

    const int contentHeight = static_cast<int>(rows.size()) * rowHeight;
    const int viewportHeight = std::max(0, getHeight() - headerHeight);
    if (contentHeight > viewportHeight && viewportHeight > 0)
    {
        const int trackX = getX() + getWidth() - 7;
        renderer.fillRect(
            { trackX, getY() + headerHeight, 5, viewportHeight },
            DefaultTheme.mutedSurface);

        const int thumbHeight = std::max(20,
            viewportHeight * viewportHeight / contentHeight);
        const int maxScroll = contentHeight - viewportHeight;
        const int thumbTravel = viewportHeight - thumbHeight;
        const int thumbY = getY() + headerHeight +
            (maxScroll > 0 ? scrollOffset * thumbTravel / maxScroll : 0);
        renderer.fillRect(
            { trackX, thumbY, 5, thumbHeight },
            DefaultTheme.darkSecondaryText);
    }

    //--------------------------------------------------
    // Outer Border
    //--------------------------------------------------

    renderer.drawRect(
        {
            getX(),
            getY(),
            getWidth(),
            getHeight()
        },
        DefaultTheme.border);

    renderer.popClip();
}

void DataGrid::addRow(
    const std::vector<std::string>& values,
    int dataIndex)
{
    rows.push_back({ values, dataIndex });
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
        selectedRow >= static_cast<int>(rows.size()))
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
    const int viewportHeight = std::max(0, getHeight() - headerHeight);
    const int maxScroll = std::max(0,
        static_cast<int>(rows.size()) * rowHeight - viewportHeight);
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
