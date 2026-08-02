#pragma once
#pragma once

#include "Panel.h"

class SidebarPanel : public Panel
{
public:
    SidebarPanel();

    void render(Renderer& renderer) override;
};