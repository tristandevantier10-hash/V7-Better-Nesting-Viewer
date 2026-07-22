#pragma once

#include <vector>
#include "Panel.h"
#include "NestingEngine.h"
#include <string>

class Renderer;

enum class ViewMode
{
    FitWidth,
    FitAll
};

class SheetPreviewPanel : public Panel
{
public:

    SheetPreviewPanel();

    void setSheets(
        const std::vector<Sheet>& sheets);

    void setViewMode(ViewMode mode);

    void render(Renderer& renderer) override;

    void update(const SDL_Event& e) override;

    void drawToolbar(Renderer& renderer, const SDL_Rect& panel);

    void drawButton(
        Renderer& renderer,
        const SDL_Rect& rect,
        const std::string& text,
        bool active);

private:

    void renderFitWidth(Renderer& renderer);

    void renderFitAll(Renderer& renderer);

    std::vector<Sheet> sheets;

    float scrollY = 0;

    float calculateScale() const;

    float calculateContentHeight() const;

    float maxScrollY = 0;

    float zoom = 1.0f;

    SDL_Rect fitWidthButton{};

    SDL_Rect fitAllButton{};

    SDL_Rect zoomInButton{};

    SDL_Rect zoomOutButton{};

    ViewMode viewMode = ViewMode::FitWidth;
};
