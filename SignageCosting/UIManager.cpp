#include "UIManager.h"

UIManager::UIManager()
{}

void UIManager::setScreen(Screen* screen)
{
    if (screen == currentScreen)
        return;

    currentScreen = screen;
    if (currentScreen)
        currentScreen->onActivated();
}

void UIManager::update(const SDL_Event& e)
{
    if (currentScreen)
    {
        currentScreen->update(e);
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

void UIManager::tick(float deltaTime)
{
    if (currentScreen)
    {
        currentScreen->tick(deltaTime);
    }
}
