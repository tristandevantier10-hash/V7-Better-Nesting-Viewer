#include "Toolbar.h"
#include "Renderer.h"
#include "HorizontalLayout.h"
#include "iostream"

Toolbar::Toolbar()
{
    setPosition(0, 60);

    setSize(1200, 45);
    setLayout(std::make_unique<HorizontalLayout>());

    newJobButton = std::make_shared<Button>();
    saveButton = std::make_shared<Button>();
    calculateButton = std::make_shared<Button>();
    printButton = std::make_shared<Button>();
    exportButton = std::make_shared<Button>();
    settingsButton = std::make_shared<Button>();

    newJobButton->setText("New Job");
    saveButton->setText("Save");
    calculateButton->setText("Calculate");
    printButton->setText("Print");
    exportButton->setText("Export");
    settingsButton->setText("Settings");

    addLayoutElement(newJobButton, SizePolicy::Fixed, 120);

    addLayoutElement(saveButton, SizePolicy::Fixed, 120);

    addLayoutElement(calculateButton, SizePolicy::Fixed, 120);

    addLayoutElement(printButton, SizePolicy::Fixed, 120);

    addLayoutElement(exportButton, SizePolicy::Fixed, 120);

    addLayoutElement(settingsButton, SizePolicy::Fixed, 120);
}

void Toolbar::setNewJobCallback(std::function<void()> callback)
{
    newJobButton->setOnClick(callback);
}

void Toolbar::setPrintCallback(std::function<void()> callback)
{
    printButton->setOnClick(callback);
}

void Toolbar::setExportCallback(std::function<void()> callback)
{
    exportButton->setOnClick(callback);
}

void Toolbar::setSaveCallback(std::function<void()> callback)
{
    saveButton->setOnClick(callback);
}

void Toolbar::setCalculateCallback(std::function<void()> callback)
{
    calculateButton->setOnClick(
        [callback]()
        {
            std::cout << "Toolbar callback fired\n";

            callback();

            std::cout << "Toolbar callback returned\n";
        });
}

void Toolbar::setSettingsCallback(std::function<void()> callback)
{
    settingsButton->setOnClick(callback);
}

void Toolbar::render(Renderer& renderer)
{
    Panel::render(renderer);
}