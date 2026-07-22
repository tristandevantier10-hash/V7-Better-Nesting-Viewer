#include "Header.h"
#include "Renderer.h"
#include "Theme.h"
#include "HorizontalLayout.h"

Header::Header()
{
    setPosition(0, 0);
    setSize(1200, 60);

    setLayout(std::make_unique<HorizontalLayout>());

    companyLabel = std::make_shared<Label>();
    titleLabel = std::make_shared<Label>();
    userLabel = std::make_shared<Label>();

    companyLabel->setText("E&G SIGNS");
    titleLabel->setText("SIGNAGE COSTING");
    userLabel->setText("ADMIN");

    addLayoutElement(companyLabel, SizePolicy::Fixed, 250);
    addLayoutElement(titleLabel, SizePolicy::Fill);
    addLayoutElement(userLabel, SizePolicy::Fixed, 150);
}

void Header::render(Renderer& renderer)
{
    Panel::render(renderer);
}