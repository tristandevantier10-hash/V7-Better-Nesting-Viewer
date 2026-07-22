#pragma once

#include "Screen.h"
#include "Window.h"
#include "InvoicePanel.h"
#include "NestingRender.h"
#include "NestingEngine.h"
#include "ResultsViewModel.h"
#include "SheetPreviewPanel.h"
#include <vector>

class Renderer;

class ResultsScreen : public Screen
{
public:

    ResultsScreen();

    Window& getWindow();

    void update(const SDL_Event& e) override;

    void render(Renderer& renderer) override;

    Toolbar& getToolbar();

    void setSheets(const std::vector<Sheet>& sheets);

    void setViewModel(const ResultsViewModel& vm);

private:

    Window window;

    std::shared_ptr<InvoicePanel> invoicePanel;

    ResultsViewModel viewModel;

    std::shared_ptr<SheetPreviewPanel> previewPanel;
};
