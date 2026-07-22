#pragma once

#include "Panel.h"
#include <string>
#include "UIElement.h"

class PropertyRow;

class FormPanel : public Panel
{
public:

    FormPanel();

    void addRow(std::shared_ptr<PropertyRow> row);

    std::shared_ptr<PropertyRow> addRow(
        const std::string& caption,
        std::shared_ptr<UIElement> control);
};