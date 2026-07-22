#pragma once

#include <SDL2/SDL.h>
#include "Screen.h"

class Renderer;

class UIManager
{
public:

    UIManager();

    void setScreen(Screen* screen);

    void update(const SDL_Event& e);

    void render(Renderer& renderer);

    void setSize(int width, int height);

private:

    Screen* currentScreen = nullptr;
};