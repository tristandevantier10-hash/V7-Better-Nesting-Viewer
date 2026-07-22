#include "StatusBar.h"
#include "Renderer.h"
#include "HorizontalLayout.h"

StatusBar::StatusBar()
{
    setPosition(0, 770);
    setSize(1200, 30);

    setLayout(std::make_unique<HorizontalLayout>());

    statusLabel = std::make_shared<Label>();
    modeLabel = std::make_shared<Label>();
    workerLabel = std::make_shared<Label>();

    statusLabel->setText("Ready");
    modeLabel->setText("Mode : Test");
    workerLabel->setText("Worker : Idle");

    addLayoutElement(statusLabel, SizePolicy::Fixed, 250);
    addLayoutElement(modeLabel, SizePolicy::Fill);
    addLayoutElement(workerLabel, SizePolicy::Fixed, 250);
}

void StatusBar::setStatus(const std::string& text)
{
    statusLabel->setText(text);
}

void StatusBar::setMode(const std::string& mode)
{
    modeLabel->setText("Mode : " + mode);
}

void StatusBar::setWorkerState(const std::string& state)
{
    workerLabel->setText("Worker : " + state);
}

void StatusBar::render(Renderer& renderer)
{
    Panel::render(renderer);
}