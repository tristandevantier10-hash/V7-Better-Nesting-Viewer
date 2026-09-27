#include "Screen.h"
#include "Renderer.h"

Screen::Screen()
{}

Screen::~Screen()
{}

void Screen::update(const SDL_Event& e)
{
    Panel::update(e);
}

void Screen::render(Renderer& renderer)
{
    Panel::render(renderer);
}

void Screen::tick(float deltaTime)
{
    Panel::tick(deltaTime);
}