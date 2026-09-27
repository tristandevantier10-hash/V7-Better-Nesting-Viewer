#include "FormPanel.h"
#include "VerticalLayout.h"
#include "PropertyRow.h"

FormPanel::FormPanel()
{
    setLayout(std::make_unique<VerticalLayout>());
}

void FormPanel::addRow(
    std::shared_ptr<PropertyRow> row)
{
    addLayoutElement(
        row,
        SizePolicy::Fixed,
        32);
}

std::shared_ptr<PropertyRow> FormPanel::addRow(
    const std::string& caption,
    std::shared_ptr<UIElement> control)
{
    auto row = std::make_shared<PropertyRow>();

    row->setCaption(caption);
    row->setControl(control);

    addRow(row);

    return row;
}

std::shared_ptr<PropertyRow> FormPanel::addCompactRow(
    const std::string& caption,
    std::shared_ptr<UIElement> control,
    int height)
{
    auto row = std::make_shared<PropertyRow>();

    row->setCaption(caption);
    row->setControl(control);

    addLayoutElement(
        row,
        SizePolicy::Fixed,
        height);

    return row;
}