#pragma once

#include <vector>
#include <memory>
#include "UIElement.h"
#include "Dock.h"
#include "Layout.h"
#include "PanelStyle.h"

class Renderer;

class Panel : public UIElement
{
public:

    Panel();

    void update(const SDL_Event& e) override;

    void render(Renderer& renderer) override;

    void add(std::shared_ptr<UIElement> element);

    void addPanel(std::shared_ptr<Panel> panel);

    void addLayoutElement(
        std::shared_ptr<UIElement> element,
        SizePolicy policy = SizePolicy::Fill,
        int size = 0);

    void addLayoutPanel(
        std::shared_ptr<Panel> panel,
        SizePolicy policy = SizePolicy::Fill,
        int size = 0);

    void setDock(Dock d);

    Dock getDock() const;

    void setLayout(std::unique_ptr<Layout> newLayout);

    void performLayout();

    void clearChildren();

    void setScrollable(bool enabled);

    bool isScrollable() const;

    void setStyle(PanelStyle newStyle)
    {
        style = newStyle;
    }

    PanelStyle getStyle() const
    {
        return style;
    }

    void setSidebarStyle(bool)
    {
        style = PanelStyle::Sidebar;
    }

    bool isSidebarStyle() const
    {
        return sidebarStyle;
    }

    void setHeaderStyle(bool)
    {
        style = PanelStyle::Header;
    }

    void setPadding(int value);

    int getPadding() const;

    void setSpacing(int value);

    int getSpacing() const;

    bool borderVisible = true;

    SDL_Color backgroundColour = { 0,0,0,0 };

    bool hasBackgroundColour = false;

    void setBorderVisible(bool value)
    {
        borderVisible = value;
    }

    bool isBorderVisible() const
    {
        return borderVisible;
    }

    void setBackgroundColour(SDL_Color colour)
    {
        backgroundColour = colour;
        hasBackgroundColour = true;
    }

    void tick(float deltaTime) override;

private:

    Dock dock = Dock::None;

protected:

    virtual void renderBackground(Renderer& renderer);

    virtual void renderChildren(Renderer& renderer);

    std::vector<std::shared_ptr<UIElement>> children;

    std::unique_ptr<Layout> layout;

    bool scrollable = false;

    int scrollY = 0;

    int scrollSpeed = 40;

    PanelStyle style = PanelStyle::Default;

    // Temporary compatibility while we migrate
    bool sidebarStyle = false;
    bool headerStyle = false;

    int padding = 0;

    int spacing = 0;

};