#pragma once

#include <SDL2/SDL.h>

#include "Panel.h"

class Renderer;

class Screen : public Panel
{
public:

    Screen();

    virtual ~Screen();

    virtual void update(const SDL_Event& e);

    virtual void render(Renderer& renderer);

};