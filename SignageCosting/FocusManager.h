#pragma once

class UIElement;

class FocusManager
{
public:

    void setFocus(UIElement* control);

    UIElement* getFocus() const;

private:

    UIElement* focused = nullptr;
};
