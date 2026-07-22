#include "NewJobScreen.h"

#include "FormPanel.h"
#include "ComboBox.h"
#include "TextBox.h"
#include "Button.h"
#include "VerticalLayout.h"
#include "MaterialDatabase.h"
#include "PropertyRow.h"
#include "iostream"
#include "ListView.h"

NewJobScreen::NewJobScreen()
{
    setLayout(std::make_unique<VerticalLayout>());

    form = std::make_shared<FormPanel>();

    material = std::make_shared<ComboBox>();

    material->setSelectionChangedCallback(
        [this](int)
        {
            loadVariants();
        });

    variant = std::make_shared<ComboBox>();

    variant->setSelectionChangedCallback(
        [this](int)
        {
            loadFormats();
        });

    formatSelector = std::make_shared<ComboBox>();

    width = std::make_shared<TextBox>();
    height = std::make_shared<TextBox>();
    quantity = std::make_shared<TextBox>();

    //=========================================
    // NEW CONTROLS
    //=========================================

    itemList = std::make_shared<ListView>();

    itemList->setSelectionChangedCallback(
        [this](int index)
        {
            loadItem(index);
        });

    addItemButton = std::make_shared<Button>();
    addItemButton->setText("Add Item");

    removeItemButton = std::make_shared<Button>();
    removeItemButton->setText("Remove Item");

    addItemButton->setOnClick(
        [this]()
        {
            JobItem item = createItem();

            std::string description =
                std::to_string(currentJob.items.size()) +
                ". " +
                item.material.id +
                " | " +
                item.material.variants[item.variantIndex].label +
                " | " +
                std::to_string((int)item.width) +
                " x " +
                std::to_string((int)item.height) +
                " | Qty " +
                std::to_string(item.quantity);

            if (editingIndex == -1)
            {
                if (activeJob)
                {
                    activeJob->addItem(item);

                    std::cout
                        << "After Add Item: "
                        << activeJob->items.size()
                        << "\n";

                }
                else
                {
                    currentJob.addItem(item);
                }

                itemList->addItem(description);
            }
            else
            {
                if (activeJob)
                {
                    activeJob->items[editingIndex] = item;
                }

                itemList->setItem(
                    editingIndex,
                    description);

                editingIndex = -1;

                addItemButton->setText("Add Item");
            }

            width->setText("");
            height->setText("");
            quantity->setText("");

            width->setFocused(true);
        });

    removeItemButton->setOnClick(
        [this]()
        {
            int index = itemList->getSelectedIndex();

            if (!activeJob)
                return;

            if (index < 0 ||
                index >= static_cast<int>(activeJob->items.size()))
            {
                return;
            }

            itemList->removeItem(index);

            activeJob->items.erase(
                activeJob->items.begin() + index);
        });

    //=========================================
    // CALCULATE BUTTON
    //=========================================

    calculateButton = std::make_shared<Button>();

    calculateButton->setText("Calculate");

    calculateButton->setOnClick(
        [this]()
        {
            calculate();
        });

    //=========================================
    // FORM
    //=========================================

    materialRow =
        form->addRow(
            "Material",
            material);

    variantRow =
        form->addRow(
            "Variant",
            variant);

    formatRow =
        form->addRow(
            "Format",
            formatSelector);

    form->addRow(
        "Width (mm)",
        width);

    form->addRow(
        "Height (mm)",
        height);

    form->addRow(
        "Quantity",
        quantity);

    //=========================================
    // LAYOUT
    //=========================================

    addLayoutElement(
        form,
        SizePolicy::Fill);

    addLayoutElement(
        addItemButton,
        SizePolicy::Fixed,
        45);

    addLayoutElement(
        removeItemButton,
        SizePolicy::Fixed,
        45);

    addLayoutElement(
        itemList,
        SizePolicy::Fill);

    addLayoutElement(
        calculateButton,
        SizePolicy::Fixed,
        45);

    formatRow->setVisible(false);
}

void NewJobScreen::calculate()
{
    if (!activeJob)
        return;

    if (activeJob->items.empty())
    {
        activeJob->addItem(createItem());
    }

    if (onCalculate)
        onCalculate();
}

void NewJobScreen::setCalculateCallback(
    std::function<void()> callback)
{
    onCalculate = callback;
}

JobItem NewJobScreen::createItem() const
{
    JobItem item;

    item.material =
        MaterialDatabase::get(
            material->getSelectedText());

    item.width =
        std::stod(width->getText());

    item.height =
        std::stod(height->getText());

    item.quantity =
        std::stoi(quantity->getText());

    item.variantIndex =
        variant->getSelectedIndex();

    if (item.variantIndex < 0 ||
        item.variantIndex >= static_cast<int>(item.material.variants.size()))
    {
        return JobItem();
    }

    const MaterialVariant& selectedVariant =
        item.material.variants[item.variantIndex];

    std::cout << "\n===== FORMAT DEBUG =====\n";
    std::cout << "Roll formats : "
        << selectedVariant.roll_widths.size()
        << "\n";

    std::cout << "Sheet formats: "
        << selectedVariant.sheet_formats.size()
        << "\n";

    //==================================================
    // ROLL MATERIAL
    //==================================================

    if (item.material.category == "Roll")
    {
        item.autoRoll = false;

        item.selectedRollWidth =
            std::stoi(
                formatSelector->getSelectedText());
    }

    //==================================================
    // SHEET MATERIAL
    //==================================================

    else if (item.material.category == "Sheet")
    {
        int index =
            formatSelector->getSelectedIndex();

        if (index >= 0 &&
            index < static_cast<int>(
                selectedVariant.sheet_formats.size()))
        {
            item.selectedSheetFormat =
                selectedVariant.sheet_formats[index];
        }
    }

    //==================================================
    // OTHER
    //==================================================

    else
    {
        item.autoRoll = true;
    }

    std::cout << "\n===== SELECTED FORMAT =====\n";
    std::cout << "Selected Index : "
        << formatSelector->getSelectedIndex()
        << "\n";

    std::cout << "Selected Text  : "
        << formatSelector->getSelectedText()
        << "\n";

    std::cout << "Width          : "
        << item.selectedSheetFormat.width
        << "\n";

    std::cout << "Height         : "
        << item.selectedSheetFormat.height
        << "\n";

    return item;
}

void NewJobScreen::initialiseMaterials()
{
    material->clear();
    variant->clear();

    auto materials = MaterialDatabase::getBaseMaterials();

    for (const auto& id : materials)
        material->addItem(id);

    if (!materials.empty())
    {
        auto vars =
            MaterialDatabase::get(materials[0]).variants;

        for (const auto& v : vars)
            variant->addItem(v.label);
    }

    loadVariants();

}

void NewJobScreen::loadVariants()
{
    variant->clear();

    Material materialData =
        MaterialDatabase::get(
            material->getSelectedText());

    for (const auto& v : materialData.variants)
    {
        variant->addItem(v.label);
    }

    loadFormats();
}

void NewJobScreen::loadFormats()
{
    formatSelector->clear();

    Material materialData =
        MaterialDatabase::get(
            material->getSelectedText());

    int variantIndex =
        variant->getSelectedIndex();

    if (variantIndex < 0 ||
        variantIndex >= static_cast<int>(materialData.variants.size()))
    {
        formatRow->setVisible(false);
        return;
    }

    const MaterialVariant& selectedVariant =
        materialData.variants[variantIndex];

    //==================================================
    // ROLL MATERIAL
    //==================================================

    if (materialData.category == "Roll")
    {
        formatRow->setVisible(true);

        formatRow->setCaption("Roll Width");

        for (int width : selectedVariant.roll_widths)
        {
            formatSelector->addItem(
                std::to_string(width));
        }
    }

    //==================================================
    // SHEET MATERIAL
    //==================================================

    else if (materialData.category == "Sheet")
    {
        formatRow->setVisible(true);

        formatRow->setCaption("Sheet Size");

        for (const auto& sheet : selectedVariant.sheet_formats)
        {
            formatSelector->addItem(
                std::to_string((int)sheet.width)
                + " x "
                + std::to_string((int)sheet.height)
                + " mm");
        }
    }

    //==================================================
    // UNKNOWN MATERIAL
    //==================================================

    else
    {
        formatRow->setVisible(false);
    }
}

void NewJobScreen::clearEditor()
{
    editingIndex = -1;

    itemList->clear();

    width->setText("");
    height->setText("");
    quantity->setText("");

    addItemButton->setText("Add Item");
}

void NewJobScreen::loadItem(int index)
{
    if (!activeJob)
        return;

    if (index < 0 ||
        index >= static_cast<int>(activeJob->items.size()))
    {
        return;
    }

    editingIndex = index;

    addItemButton->setText("Update Item");

    const JobItem& item =
        activeJob->items[index];

    //=========================
    // Material
    //=========================

    auto materials =
        MaterialDatabase::getBaseMaterials();

    for (int i = 0; i < static_cast<int>(materials.size()); i++)
    {
        if (materials[i] == item.material.id)
        {
            material->setSelectedIndex(i);
            break;
        }
    }

    loadVariants();

    //=========================
    // Variant
    //=========================

    variant->setSelectedIndex(
        item.variantIndex);

    loadFormats();

    //=========================
    // Dimensions
    //=========================

    width->setText(
        std::to_string((int)item.width));

    height->setText(
        std::to_string((int)item.height));

    quantity->setText(
        std::to_string(item.quantity));
}

void NewJobScreen::setJob(Job* job)
{
    activeJob = job;
}
