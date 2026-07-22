#include "UIManager.h"
#include <iostream>


UIManager::UIManager()
{}

void UIManager::setScreen(Screen* screen)
{
    currentScreen = screen;
}

void UIManager::update(const SDL_Event& e)
{
    if (currentScreen)
    {
        if (e.type == SDL_MOUSEWHEEL)
        {
            std::cout
                << "UIManager forwarding wheel\n";
        }

        currentScreen->update(e);
    }
    else
    {
        if (e.type == SDL_MOUSEWHEEL)
        {
            std::cout
                << "NO CURRENT SCREEN\n";
        }
    }
}

void UIManager::render(Renderer& renderer)
{
    if (currentScreen)
    {
        currentScreen->render(renderer);
    }
}

void UIManager::setSize(int width, int height)
{
    if (!currentScreen)
        return;

    currentScreen->setPosition(0, 0);
    currentScreen->setSize(width, height);
}