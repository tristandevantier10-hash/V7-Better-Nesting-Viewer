#pragma once

#include <memory>
#include <string>
#include "Panel.h"
#include "Label.h"
#include "TextBox.h"
#include "HorizontalLayout.h"

class Renderer;

class PropertyRow : public Panel
{
public:

    PropertyRow();

    PropertyRow(
        const std::string& caption,
        const std::string& value = "");

    void setCaption(const std::string& text);

    void setValue(const std::string& text);

    void render(Renderer& renderer) override;

    void setControl(std::shared_ptr<UIElement> control);

private:

    std::shared_ptr<Label> caption;

    std::shared_ptr<UIElement> valueControl;
};
