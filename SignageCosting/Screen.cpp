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