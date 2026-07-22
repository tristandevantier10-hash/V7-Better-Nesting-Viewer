#include "ResultsScreen.h"
#include "Renderer.h"
#include <iostream>
#include <windows.h>

ResultsScreen::ResultsScreen()
{

    invoicePanel = std::make_shared<InvoicePanel>();

    previewPanel = std::make_shared<SheetPreviewPanel>();

    window.addContentPanel(
        invoicePanel,
        SizePolicy::Fixed,
        340);

    window.addContentPanel(
        previewPanel,
        SizePolicy::Fill);

}

void ResultsScreen::update(const SDL_Event& e)
{
    if (e.type == SDL_MOUSEWHEEL)
    {
        std::cout
            << "ResultsScreen received wheel\n";
    }

    window.update(e);
}

void ResultsScreen::render(Renderer& renderer)
{
    window.render(renderer);
}

void ResultsScreen::setSheets(
    const std::vector<Sheet>& sheets)
{
    previewPanel->setSheets(sheets);
}

Toolbar& ResultsScreen::getToolbar()
{
    return window.getToolbar();
}

void ResultsScreen::setViewModel(
    const ResultsViewModel& vm)
{
    viewModel = vm;

    invoicePanel->setViewModel(viewModel.invoice);
}

Window& ResultsScreen::getWindow()
{
    return window;
}