#pragma once

#include "Panel.h"
#include <string>

class CardPanel : public Panel
{
public:

    CardPanel();

    void setTitle(const std::string& text);

    const std::string& getTitle() const;

    void render(Renderer& renderer) override;

private:

    std::string title;

    int headerHeight = 40;
};