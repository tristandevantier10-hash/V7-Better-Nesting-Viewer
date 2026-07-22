#pragma once

#include "Panel.h"
#include "Label.h"

class Renderer;

class StatusBar : public Panel
{
public:

    StatusBar();

    void setStatus(const std::string& text);

    void setMode(const std::string& mode);

    void setWorkerState(const std::string& state);

    void render(Renderer& renderer) override;

private:

    std::shared_ptr<Label> statusLabel;

    std::shared_ptr<Label> modeLabel;

    std::shared_ptr<Label> workerLabel;
};
