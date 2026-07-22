#pragma once

#include "Screen.h"
#include "Label.h"
#include "Button.h"

class Renderer;

class JobEditorScreen : public Screen
{
public:

    JobEditorScreen();

    void update(const SDL_Event& e) override;

    void render(Renderer& renderer) override;

private:

    Label title;

    Button calculateButton;

    Button cancelButton;
};
