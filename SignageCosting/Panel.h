#pragma once

#include <vector>
#include <memory>
#include "UIElement.h"
#include "Dock.h"
#include "Layout.h"

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

private:

    Dock dock = Dock::None;

protected:

    std::vector<std::shared_ptr<UIElement>> children;

    std::unique_ptr<Layout> layout;

    bool scrollable = false;

    int scrollY = 0;

    int scrollSpeed = 40;

};