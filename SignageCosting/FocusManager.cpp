#include "FocusManager.h"
#include "UIElement.h"

void FocusManager::setFocus(UIElement* control)
{
    focused = control;
}

UIElement* FocusManager::getFocus() const
{
    return focused;
}