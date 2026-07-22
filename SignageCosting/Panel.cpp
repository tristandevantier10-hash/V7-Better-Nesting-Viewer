#include "Panel.h"
#include "Renderer.h"
#include "Theme.h"
#include "iostream"

Panel::Panel()
{
    setPosition(0, 0);

    setSize(100, 100);
}

void Panel::add(std::shared_ptr<UIElement> element)
{
    children.push_back(element);
}

void Panel::update(const SDL_Event& e)
{
    if (scrollable &&
        e.type == SDL_MOUSEWHEEL)
    {
        int mx, my;
        SDL_GetMouseState(&mx, &my);

        SDL_Point mouse =
        {
            mx,
            my
        };

        if (SDL_PointInRect(&mouse, &bounds))
        {
            scrollY -= e.wheel.y * scrollSpeed;

            if (scrollY < 0)
                scrollY = 0;
        }
    }

    for (auto& child : children)
    {
        if (child->isVisible())
        {
            child->update(e);
        }
    }
}

void Panel::render(Renderer& renderer)
{

    /*std::cout
        << "Panel "
        << getX() << ","
        << getY()
        << " "
        << getWidth()
        << "x"
        << getHeight()
        << std::endl;
        */

    renderer.fillRect(
        bounds,
        DefaultTheme.panelBackground);

    renderer.drawRect(
        bounds,
        DefaultTheme.border);

    performLayout();

    if (scrollable)
    {
        SDL_Rect clip =
        {
            getX(),
            getY(),
            getWidth(),
            getHeight()
        };

        renderer.pushClip(clip);
        renderer.pushOffset(0, -scrollY);
    }

    for (auto& child : children)
    {
        if (child->isVisible())
        {
            child->render(renderer);
        }
    }

    if (scrollable)
    {
        renderer.popOffset();
        renderer.popClip();
    }
}

void Panel::addPanel(std::shared_ptr<Panel> panel)
{
    add(panel);
}

void Panel::setDock(Dock d)
{
    dock = d;
}

Dock Panel::getDock() const
{
    return dock;
}

void Panel::setLayout(std::unique_ptr<Layout> newLayout)
{
    layout = std::move(newLayout);
}

void Panel::performLayout()
{
    if (layout)
    {
        layout->performLayout(
            getX(),
            getY(),
            getWidth(),
            getHeight());
    }
}

void Panel::addLayoutPanel(
    std::shared_ptr<Panel> panel,
    SizePolicy policy,
    int size)
{
    addPanel(panel);

    if (layout)
    {
        layout->addElement(panel, policy, size);
    }
}

void Panel::addLayoutElement(
    std::shared_ptr<UIElement> element,
    SizePolicy policy,
    int size)
{
    add(element);

    if (layout)
    {
        layout->addElement(element, policy, size);
    }
}

void Panel::clearChildren()
{
    children.clear();

    if (layout)
    {
        layout->clear();
    }
}

void Panel::setScrollable(bool enabled)
{
    scrollable = enabled;
}

bool Panel::isScrollable() const
{
    return scrollable;
}