#include "UIElement.h"

UIElement::UIElement()
{
    bounds = { 0,0,100,30 };

    visible = true;

    enabled = true;

    hovered = false;

    pressed = false;
}

UIElement::~UIElement()
{}

void UIElement::update(const SDL_Event&)
{}

void UIElement::render(Renderer&)
{}

void UIElement::setPosition(int x, int y)
{
    bounds.x = x;
    bounds.y = y;
}

void UIElement::setSize(int w, int h)
{
    bounds.w = w;
    bounds.h = h;
}

SDL_Rect UIElement::getBounds() const
{
    return bounds;
}

int UIElement::getX() const
{
    return bounds.x;
}

int UIElement::getY() const
{
    return bounds.y;
}

int UIElement::getWidth() const
{
    return bounds.w;
}

int UIElement::getHeight() const
{
    return bounds.h;
}

void UIElement::onFocus()
{}

void UIElement::onBlur()
{}

bool UIElement::hasFocus() const
{
    return focused;
}

void UIElement::setFocused(bool value)
{
    if (focused == value)
        return;

    focused = value;

    if (focused)
        onFocus();
    else
        onBlur();
}

void UIElement::setVisible(bool value)
{
    visible = value;
}

bool UIElement::isVisible() const
{
    return visible;
}