#pragma once

#include "Screen.h"
#include <memory>
#include <functional>
#include "Job.h"

class FormPanel;
class ComboBox;
class TextBox;
class Button;
class PropertyRow;
class ListView;

class NewJobScreen : public Screen
{
public:

    NewJobScreen();

    void setCalculateCallback(
        std::function<void()> callback);

    void initialiseMaterials();

    void clearEditor();

    void setJob(Job* job);

private:

    void calculate();

    void loadVariants();

    void loadFormats();

    void loadItem(int index);

    std::function<void()> onCalculate;

    std::shared_ptr<FormPanel> form;

    std::shared_ptr<PropertyRow> materialRow;
    std::shared_ptr<PropertyRow> variantRow;
    std::shared_ptr<PropertyRow> formatRow;

    std::shared_ptr<ComboBox> material;
    std::shared_ptr<ComboBox> variant;
    std::shared_ptr<ComboBox> formatSelector;

    std::shared_ptr<TextBox> width;
    std::shared_ptr<TextBox> height;
    std::shared_ptr<TextBox> quantity;

    std::shared_ptr<Button> addItemButton;
    std::shared_ptr<Button> removeItemButton;
    std::shared_ptr<Button> calculateButton;

    std::shared_ptr<ListView> itemList;

    Job currentJob;

    int editingIndex = -1;

    JobItem createItem() const;

    Job* activeJob = nullptr;
};
