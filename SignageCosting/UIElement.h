#pragma once

#include <SDL2/SDL.h>

class Renderer;

class UIElement
{
public:

    UIElement();

    virtual ~UIElement();

    virtual void update(const SDL_Event& e);

    virtual void render(Renderer& renderer);

    void setPosition(int x, int y);

    void setSize(int w, int h);

    SDL_Rect getBounds() const;

    int getX() const;
    int getY() const;
    int getWidth() const;
    int getHeight() const;

    virtual void onFocus();
    virtual void onBlur();

    bool hasFocus() const;

    void setFocused(bool value);

    void setVisible(bool value);

    bool isVisible() const;

protected:

    SDL_Rect bounds{ 0, 0, 0, 0 };

    bool visible = true;

    bool enabled = true;

    bool hovered = false;

    bool pressed = false;

    bool focused = false;
};