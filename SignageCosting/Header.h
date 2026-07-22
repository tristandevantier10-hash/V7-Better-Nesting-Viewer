#pragma once

#include "Panel.h"
#include "Label.h"

class Renderer;

class Header : public Panel
{
public:

    Header();

    void render(Renderer& renderer) override;

private:

    std::shared_ptr<Label> companyLabel;

    std::shared_ptr<Label> titleLabel;

    std::shared_ptr<Label> userLabel;
};
