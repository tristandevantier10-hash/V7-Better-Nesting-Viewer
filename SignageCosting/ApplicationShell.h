#pragma once

#include "Screen.h"

class Panel;
class NavigationPanel;

class ApplicationShell : public Screen
{
public:

    ApplicationShell();

    void setContent(std::shared_ptr<UIElement> content);

private:

    std::shared_ptr<Panel> headerPanel;

    std::shared_ptr<NavigationPanel> navigationPanel;

    std::shared_ptr<Panel> contentPanel;

    std::shared_ptr<Panel> bodyPanel;
};
