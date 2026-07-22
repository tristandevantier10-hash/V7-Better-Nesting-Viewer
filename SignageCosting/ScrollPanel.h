#pragma once

#include "Panel.h"

class Renderer;

class ScrollPanel : public Panel
{
public:

    ScrollPanel();

    void update(const SDL_Event& e) override;

    void render(Renderer& renderer) override;

private:

    int scrollY = 0;

    int scrollSpeed = 40;
};
