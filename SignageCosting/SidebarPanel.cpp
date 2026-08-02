#include "SidebarPanel.h"
#include "Renderer.h"

SidebarPanel::SidebarPanel()
{
    setSize(220, 800);
}

void SidebarPanel::render(Renderer& renderer)
{
    Panel::render(renderer);
}