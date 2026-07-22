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
        26);
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