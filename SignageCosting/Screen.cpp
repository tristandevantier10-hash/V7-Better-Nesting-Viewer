#include "Screen.h"
#include "Renderer.h"
#include "TextBox.h"
#include <algorithm>
#include <vector>

Screen::Screen()
{}

Screen::~Screen()
{}

void Screen::update(const SDL_Event& e)
{
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_TAB)
    {
        std::vector<TextBox*> textBoxes;
        collectTextBoxes(textBoxes);

        if (!textBoxes.empty())
        {
            const auto focused = std::find_if(
                textBoxes.begin(), textBoxes.end(),
                [](const TextBox* textBox) { return textBox->hasFocus(); });

            const int count = static_cast<int>(textBoxes.size());
            const bool backwards = (e.key.keysym.mod & KMOD_SHIFT) != 0;
            int nextIndex = backwards ? count - 1 : 0;

            if (focused != textBoxes.end())
            {
                const int currentIndex = static_cast<int>(focused - textBoxes.begin());
                nextIndex = (currentIndex + (backwards ? count - 1 : 1)) % count;
            }

            for (TextBox* textBox : textBoxes)
                textBox->setFocused(false);
            textBoxes[nextIndex]->setFocused(true);
            return;
        }
    }

    Panel::update(e);
}

void Screen::render(Renderer& renderer)
{
    Panel::render(renderer);
    renderOverlays(renderer);
}

void Screen::tick(float deltaTime)
{
    Panel::tick(deltaTime);
}
