#include "PropertyRow.h"
#include "Renderer.h"
#include "Metrics.h"

PropertyRow::PropertyRow()
{
    setSize(250, Metrics::ControlHeight);

    setPadding(0);
    setBorderVisible(false);
    setBackgroundColour({ 255, 255, 255, 255 });

    auto rowLayout = std::make_unique<HorizontalLayout>();
    rowLayout->setCenterChildrenVertically(true);
    setLayout(std::move(rowLayout));

    caption =
        std::make_shared<Label>();

    addLayoutElement(
        caption,
        SizePolicy::Fixed,
        144);
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

    label->setText(text);
}

void PropertyRow::render(Renderer& renderer)
{
    Panel::render(renderer);
}
