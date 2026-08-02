#include "DataGrid.h"
#include "Renderer.h"
#include "label.h"  

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

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    if (mouseX >= getX() &&
        mouseX < getX() + getWidth() &&
        mouseY >= getY() &&
        mouseY < getY() + getHeight())
    {
        int localY = mouseY - getY();

        if (localY > headerHeight)
        {
            hoveredRow =
                (localY - headerHeight) / rowHeight;

            if (hoveredRow >= static_cast<int>(rows.size()))
                hoveredRow = -1;
        }
    }

    if (e.type == SDL_MOUSEBUTTONDOWN &&
        e.button.button == SDL_BUTTON_LEFT)
    {
        if (hoveredRow != -1)
            selectedRow = hoveredRow;
    }
}

void DataGrid::render(Renderer& renderer)
{

    if (!visible)
        return;

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
        { 255,255,255,255 });

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
        { 245,245,245,255 });

    //--------------------------------------------------
    // Column titles
    //--------------------------------------------------

    int x = getX();

    for (const auto& column : columns)
    {
        renderer.drawText(
            column.title,
            x + 10,
            getY() + 10);

        x += column.width;

        renderer.drawLine(
            x,
            getY(),
            x,
            getY() + headerHeight,
            { 220,220,220,255 });
    }

    //--------------------------------------------------
    // Bottom header line
    //--------------------------------------------------

    renderer.drawLine(
        getX(),
        getY() + headerHeight,
        getX() + getWidth(),
        getY() + headerHeight,
        { 220,220,220,255 });

    //--------------------------------------------------
    // Rows
    //--------------------------------------------------

    int y = getY() + headerHeight;

    for (size_t rowIndex = 0;
        rowIndex < rows.size();
        ++rowIndex)
    {

        SDL_Color rowColour;

        if ((int)rowIndex == selectedRow)
        {
            rowColour = DefaultTheme.accent;
        }
        else if ((int)rowIndex == hoveredRow)
        {
            rowColour = { 240,244,248,255 };
        }
        else
        {
            rowColour =
                (rowIndex % 2 == 0)
                ? SDL_Color{ 255,255,255,255 }
            : SDL_Color{ 250,250,250,255 };
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
            SDL_Color textColour =
                ((int)rowIndex == selectedRow)
                ? SDL_Color{ 255,255,255,255 }
            : DefaultTheme.darkText;

            if (col == 0)
            {
                renderer.drawText(
                    rows[rowIndex].values[col],
                    x + 10,
                    y + 6,
                    LabelStyle::Normal,
                    DefaultTheme.darkText);

                renderer.drawText(
                    "Cash Customer",
                    x + 10,
                    y + 28,
                    LabelStyle::Small,
                    DefaultTheme.accent);
            }
            else
            {
                renderer.drawText(
                    rows[rowIndex].values[col],
                    x + 10,
                    y + 18,
                    LabelStyle::Normal,
                    textColour);
            }

            // Draw the separator at the right edge of this column
            renderer.drawLine(
                x + columns[col].width,
                y,
                x + columns[col].width,
                y + rowHeight,
                { 230,230,230,255 });

            x += columns[col].width;
        }

        y += rowHeight;
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
        { 220,220,220,255 });
}

void DataGrid::addRow(
    const std::vector<std::string>& values)
{
    rows.push_back({ values });
}

void DataGrid::clear()
{
    rows.clear();
}