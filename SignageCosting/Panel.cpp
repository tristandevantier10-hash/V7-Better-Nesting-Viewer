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

void Panel::renderBackground(Renderer& renderer)
{

    if (hasBackgroundColour)
    {
        renderer.fillRoundedRect(
            {
                getX(),
                getY(),
                getWidth(),
                getHeight()
            },
            backgroundColour,
            8);

        return;
    }

    switch (style)
    {
    case PanelStyle::Sidebar:

        renderer.fillRect(
            bounds,
            SDL_Color{ 15,15,15,255 });

        break;

    case PanelStyle::Header:

        renderer.fillRect(
            bounds,
            DefaultTheme.headerBackground);

        if (borderVisible)
        {
            renderer.drawRect(
                bounds,
                DefaultTheme.border);
        }

        break;

    case PanelStyle::Card:

        renderer.fillRect(
            bounds,
            SDL_Color{ 255,255,255,255 });

        if (borderVisible)
        {
            renderer.drawRect(
                bounds,
                SDL_Color{ 220,220,220,255 });
        }

        break;

    default:

        renderer.fillRect(
            bounds,
            DefaultTheme.panelBackground);

        if (borderVisible)
        {
            renderer.drawRect(
                bounds,
                DefaultTheme.border);
        }

        break;
    }
}

void Panel::renderChildren(Renderer& renderer)
{
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

void Panel::render(Renderer& renderer)
{
    performLayout();

    renderBackground(renderer);

    renderChildren(renderer);
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
    // First calculate this panel's direct children.
    if (layout)
    {
        layout->performLayout(
            getX(),
            getY(),
            getWidth(),
            getHeight(),
            getPadding());
    }

    // Then recursively calculate the layouts
    // of any child panels.
    for (auto& child : children)
    {
        if (!child->isVisible())
            continue;

        auto panel =
            std::dynamic_pointer_cast<Panel>(child);

        if (panel)
        {
            panel->performLayout();
        }
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

void Panel::setPadding(int value)
{
    padding = value;
}

int Panel::getPadding() const
{
    return padding;
}

void Panel::setSpacing(int value)
{
    if (layout)
    {
        layout->setSpacing(value);
    }
}

int Panel::getSpacing() const
{
    if (layout)
    {
        return layout->getSpacing();
    }

    return 0;
}

void Panel::tick(float deltaTime)
{
    for (auto& child : children)
    {
        child->tick(deltaTime);
    }
}