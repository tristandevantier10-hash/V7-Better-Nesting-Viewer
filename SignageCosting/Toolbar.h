#pragma once

#include "Panel.h"
#include "Button.h"
#include <functional>

class Renderer;

class Toolbar : public Panel
{
public:

    void setNewJobCallback(std::function<void()> callback);

    void setPrintCallback(std::function<void()> callback);

    void setExportCallback(std::function<void()> callback);

    void setSaveCallback(std::function<void()> callback);

	void setCalculateCallback(std::function<void()> callback);

    void setSettingsCallback(std::function<void()> callback);

    Toolbar();

    void render(Renderer& renderer) override;

protected:

    std::shared_ptr<Button> newJobButton;

    std::shared_ptr<Button> saveButton;

    std::shared_ptr<Button> calculateButton;

    std::shared_ptr<Button> printButton;

    std::shared_ptr<Button> exportButton;

    std::shared_ptr<Button> settingsButton;
};
