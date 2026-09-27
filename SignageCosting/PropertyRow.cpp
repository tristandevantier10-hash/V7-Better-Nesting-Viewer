#include "PropertyRow.h"
#include "Renderer.h"
#include "iostream"

PropertyRow::PropertyRow()
{
    setSize(250, 32);

    setPadding(0);
    setBorderVisible(false);
    setBackgroundColour({ 255, 255, 255, 255 });

    setLayout(
        std::make_unique<HorizontalLayout>());

    caption =
        std::make_shared<Label>();

    addLayoutElement(
        caption,
        SizePolicy::Fixed,
        110);
}

void PropertyRow::setControl(std::shared_ptr<UIElement> control)
{
    valueControl = control;

    addLayoutElement(
        control,
        SizePolicy::Fill);
}

PropertyRow::PropertyRow(
    const std::string& captionText,
    const std::string& valueText)
    : PropertyRow()
{
    setCaption(captionText);

    auto value =
        std::make_shared<Label>();

    value->setText(valueText);

    setControl(value);
}

void PropertyRow::setCaption(const std::string& text)
{
    caption->setText(text);
}

void PropertyRow::setValue(const std::string& text)
{
    auto label =
        std::dynamic_pointer_cast<Label>(valueControl);

    if (!label)
    {
        label = std::make_shared<Label>();

        setControl(label);
    }

    std::cout << "Setting PropertyRow to: " << text << std::endl;

    label->setText(text);
}

void PropertyRow::render(Renderer& renderer)
{
    Panel::render(renderer);
}