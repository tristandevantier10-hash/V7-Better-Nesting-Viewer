#pragma once

#include "Panel.h"
#include <string>
#include <functional>

class Renderer;

class TextBox : public Panel
{
public:

    TextBox();

    void update(const SDL_Event& e) override;

    void render(Renderer& renderer) override;

    void setText(const std::string& value);

    const std::string& getText() const;

    void setTextChangedCallback(
        std::function<void(const std::string&)> callback);

private:

    std::string text;

    bool focused = false;

    Uint32 lastBlink = 0;

    bool showCaret = true;

    std::function<void(const std::string&)> textChangedCallback;

};
