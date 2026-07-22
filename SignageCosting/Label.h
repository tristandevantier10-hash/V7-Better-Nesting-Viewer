#pragma once

#include "UIElement.h"
#include <string>

class Renderer;

class Label : public UIElement
{
public:

    Label();

    void setText(const std::string& text);

    const std::string& getText() const;

    void render(Renderer& renderer) override;

private:

    std::string text;
};