#include "Label.h"
#include "Renderer.h"
#include "TextRenderer.h"
#include "iostream"

Label::Label()
{}

void Label::setText(const std::string& value)
{
    text = value;
}

const std::string& Label::getText() const
{
    return text;
}

void Label::render(Renderer& renderer)
{

    /*std::cout
        << "Label '" << text
        << "' at "
        << getX()
        << ", "
        << getY()
        << std::endl;
        */

    if (!visible)
        return;

    renderer.drawText(
        text,
        getX(),
        getY());
}