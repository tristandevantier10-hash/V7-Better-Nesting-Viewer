#include "Window.h"
#include "VerticalLayout.h"
#include "HorizontalLayout.h"
#include "Renderer.h"
#include "iostream"

Window::Window()
{
    setPosition(0, 0);
    setSize(1200, 800);

    setLayout(std::make_unique<VerticalLayout>());

    header = std::make_shared<Header>();
    toolbar = std::make_shared<Toolbar>();
    content = std::make_shared<Panel>();
    content->setLayout(std::make_unique<HorizontalLayout>());
    statusBar = std::make_shared<StatusBar>();

    addPanel(header);
    addPanel(toolbar);
    addPanel(content);
    addPanel(statusBar);

    this->layout->addElement(header, SizePolicy::Fixed, 60);
    this->layout->addElement(toolbar, SizePolicy::Fixed, 45);
    this->layout->addElement(content, SizePolicy::Fill);
    this->layout->addElement(statusBar, SizePolicy::Fixed, 30);
}

void Window::addContentPanel(
    std::shared_ptr<Panel> panel,
    SizePolicy policy,
    int size)
{
    content->addLayoutPanel(panel, policy, size);

}

void Window::update(const SDL_Event& e)
{
    if (e.type == SDL_MOUSEWHEEL)
        std::cout << "Window received wheel\n";

    Panel::update(e);
}

void Window::render(Renderer& renderer)
{
    Panel::render(renderer);
}

void Window::setWindowSize(
    int width,
    int height)
{
    setPosition(0, 0);
    setSize(width, height);
}

Toolbar& Window::getToolbar()
{
    return *toolbar;
}