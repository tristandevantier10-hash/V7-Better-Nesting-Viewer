#pragma once

#include "Panel.h"
#include "Header.h"
#include "Toolbar.h"
#include "StatusBar.h"
#include <memory>

class Renderer;

class Window : public Panel
{
public:

    Window();

    void setWindowSize(
        int width,
        int height);

    void addContentPanel(
        std::shared_ptr<Panel> panel,
        SizePolicy policy,
        int size = 0);

    void update(const SDL_Event& e) override;

    void render(Renderer& renderer) override;

    Toolbar& getToolbar();

protected:

    std::shared_ptr<Header> header;

    std::shared_ptr<Toolbar> toolbar;

    std::shared_ptr<Panel> content;

    std::shared_ptr<StatusBar> statusBar;

};
