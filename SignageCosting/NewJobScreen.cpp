#include "NewJobScreen.h"

#include "FormPanel.h"
#include "ComboBox.h"
#include "TextBox.h"
#include "Button.h"
#include "VerticalLayout.h"
#include "HorizontalLayout.h"
#include "NavigationPanel.h"
#include "Metrics.h"
#include "MaterialDatabase.h"
#include "PropertyRow.h"
#include "DataGrid.h"
#include "Panel.h"
#include "Label.h"
#include "iostream"

NewJobScreen::NewJobScreen()
{
    //==================================================
    // Root
    //==================================================

    setLayout(std::make_unique<VerticalLayout>());

    auto contentPanel = std::make_shared<Panel>();
    contentPanel->setLayout(
        std::make_unique<HorizontalLayout>());

    addLayoutElement(
        contentPanel,
        SizePolicy::Fill);

    //==================================================
    // Sidebar
    //==================================================

    auto sidebar = std::make_shared<NavigationPanel>();

    sidebar->setLayout(
        std::make_unique<VerticalLayout>());

    sidebar->setSidebarStyle(true);
    sidebar->setPadding(Metrics::SidebarPadding);

    contentPanel->addLayoutElement(
        sidebar,
        SizePolicy::Fixed,
        Metrics::SidebarWidth);

    sidebar->addItem(
        "JOB INPUT",
        []() {});

    struct InvisibleSpacer : public Panel
    {
        void render(Renderer& renderer) override
        {
            performLayout();
            renderChildren(renderer);
        }
    };

    auto spacer =
        std::make_shared<InvisibleSpacer>();

    spacer->setBorderVisible(false);

    sidebar->addLayoutElement(
        spacer,
        SizePolicy::Fill);

    //==================================================
    // Right Side
    //==================================================

    auto rightContainer =
        std::make_shared<Panel>();

    rightContainer->setLayout(
        std::make_unique<VerticalLayout>());

    rightContainer->setStyle(
        PanelStyle::Card);

    rightContainer->setBorderVisible(false);
    rightContainer->setPadding(30);
    rightContainer->setSpacing(20);

    contentPanel->addLayoutElement(
        rightContainer,
        SizePolicy::Fill);

    //==================================================
    // Header
    //==================================================

    auto pageHeader =
        std::make_shared<Panel>();

    pageHeader->setLayout(
        std::make_unique<HorizontalLayout>());

    pageHeader->setStyle(
        PanelStyle::Card);

    pageHeader->setBorderVisible(false);

    rightContainer->addLayoutElement(
        pageHeader,
        SizePolicy::Fixed,
        100);

    auto titlePanel =
        std::make_shared<Panel>();

    titlePanel->setLayout(
        std::make_unique<VerticalLayout>());

    titlePanel->setStyle(
        PanelStyle::Card);

    titlePanel->setBorderVisible(false);
    titlePanel->setSpacing(4);

    pageHeader->addLayoutElement(
        titlePanel,
        SizePolicy::Fill);

    auto title =
        std::make_shared<Label>();

    title->setText("Job Input");
    title->setStyle(LabelStyle::Heading);

    titlePanel->addLayoutElement(
        title,
        SizePolicy::Fixed,
        36);

    auto subtitle =
        std::make_shared<Label>();

    subtitle->setText(
        "Enter the materials and dimensions for this job");

    subtitle->setStyle(
        LabelStyle::Small);

    subtitle->setTextTheme(
        TextTheme::DarkSecondary);

    titlePanel->addLayoutElement(
        subtitle,
        SizePolicy::Fixed,
        24);

    //==================================================
    // Selected Customer Card
    //==================================================

    customerCard =
        std::make_shared<Panel>();

    customerCard->setLayout(
        std::make_unique<VerticalLayout>());

    auto customerHeader =
        std::make_shared<Panel>();

    customerHeader->setLayout(
        std::make_unique<HorizontalLayout>());

    customerHeader->setBorderVisible(false);
    customerHeader->setPadding(0);
    customerHeader->setSpacing(8);

    customerTitle =
        std::make_shared<Label>();

    customerTitle->setText(
        "Selected Customer");

    customerTitle->setStyle(
        LabelStyle::Small);

    customerHeader->addLayoutElement(
        customerTitle,
        SizePolicy::Fill);

    customerType =
        std::make_shared<Label>();

    customerType->setText(
        "");

    customerType->setStyle(
        LabelStyle::Small);

    customerType->setTextTheme(
        TextTheme::DarkSecondary);

    customerHeader->addLayoutElement(
        customerType,
        SizePolicy::Fixed,
        110);

    customerCard->addLayoutElement(
        customerHeader,
        SizePolicy::Fixed,
        30);

    customerCompany =
        std::make_shared<Label>();

    customerCompany->setText(
        "No customer selected");

    customerCompany->setStyle(
        LabelStyle::Normal);

    customerCard->addLayoutElement(
        customerCompany,
        SizePolicy::Fixed,
        15);

    customerContact =
        std::make_shared<Label>();

    customerContact->setText("");

    customerContact->setStyle(
        LabelStyle::Normal);

    customerCard->addLayoutElement(
        customerContact,
        SizePolicy::Fixed,
        15);


    customerPhone =
        std::make_shared<Label>();

    customerPhone->setText("");

    customerPhone->setStyle(
        LabelStyle::Normal);

    customerCard->addLayoutElement(
        customerPhone,
        SizePolicy::Fixed,
        15);

    customerCard->setStyle(
        PanelStyle::Card);

    customerCard->setPadding(10);
    customerCard->setSpacing(1);

    pageHeader->addLayoutElement(
        customerCard,
        SizePolicy::Fixed,
        430);

    //==================================================
    // Main Workspace
    //==================================================

    auto workspace =
        std::make_shared<Panel>();

    workspace->setLayout(
        std::make_unique<VerticalLayout>());

    workspace->setStyle(
        PanelStyle::Card);

    workspace->setBorderVisible(false);
    workspace->setPadding(0);
    workspace->setSpacing(18);

    rightContainer->addLayoutElement(
        workspace,
        SizePolicy::Fill);

    //==================================================
    // Initialise Existing Controls
    //==================================================

    form =
        std::make_shared<FormPanel>();

    form->setStyle(PanelStyle::Card);
    form->setBorderVisible(false);
    form->setBackgroundColour({ 255,255,255,255 });
    form->setPadding(0);
    form->setSpacing(8);

    material =
        std::make_shared<ComboBox>();

    material->setSelectionChangedCallback(
        [this](int)
        {
            loadVariants();
        });

    variant =
        std::make_shared<ComboBox>();

    variant->setSelectionChangedCallback(
        [this](int)
        {
            loadFormats();
        });

    formatSelector =
        std::make_shared<ComboBox>();

    width =
        std::make_shared<TextBox>();

    height =
        std::make_shared<TextBox>();

    quantity =
        std::make_shared<TextBox>();

    //==================================================
    // Form Rows
    //==================================================

    materialRow =
        form->addCompactRow(
            "Material",
            material,
            34);

    variantRow =
        form->addCompactRow(
            "Variant",
            variant,
            34);

    formatRow =
        form->addCompactRow(
            "Format",
            formatSelector,
            34);

    form->addCompactRow(
        "Width (mm)",
        width,
        34);

    form->addCompactRow(
        "Height (mm)",
        height,
        34);

    form->addCompactRow(
        "Quantity",
        quantity,
        34);

    //==================================================
    // Material Editor Header
    //==================================================

    auto editorHeader =
        std::make_shared<Panel>();

    editorHeader->setLayout(
        std::make_unique<HorizontalLayout>());

    editorHeader->setStyle(
        PanelStyle::Card);

    editorHeader->setBorderVisible(false);
    editorHeader->setPadding(0);
    editorHeader->setSpacing(8);

    auto editorTitle =
        std::make_shared<Label>();

    editorTitle->setText(
        "Material");

    editorTitle->setStyle(
        LabelStyle::Heading);

    editorHeader->addLayoutElement(
        editorTitle,
        SizePolicy::Fill,
        30);

    auto editorHint =
        std::make_shared<Label>();

    editorHint->setText(
        "Configure the material for this item");

    editorHint->setStyle(
        LabelStyle::Small);

    editorHint->setTextTheme(
        TextTheme::DarkSecondary);

    editorHeader->addLayoutElement(
        editorHint,
        SizePolicy::Fixed,
        230);

    workspace->addLayoutElement(
        editorHeader,
        SizePolicy::Fixed,
        32);

    //==================================================
    // Create Action Column
    //==================================================

    auto actionColumn =
        std::make_shared<Panel>();

    actionColumn->setLayout(
        std::make_unique<VerticalLayout>());

    auto actionSpacer =
        std::make_shared<InvisibleSpacer>();

    actionSpacer->setBorderVisible(false);

    actionColumn->addLayoutElement(
        actionSpacer,
        SizePolicy::Fill);

    actionColumn->setBorderVisible(false);
    actionColumn->setBackgroundColour(
        { 255, 255, 255, 255 });
    actionColumn->setSpacing(12);

    //==================================================
    // Input Area
    //==================================================

    auto inputArea =
        std::make_shared<Panel>();

    inputArea->setBackgroundColour(
        { 255,255,255,255 });

    inputArea->setLayout(
        std::make_unique<HorizontalLayout>());

    inputArea->setBorderVisible(false);
    inputArea->setSpacing(20);

    inputArea->addLayoutElement(
        form,
        SizePolicy::Fixed,
        500);

    inputArea->addLayoutElement(
        actionColumn,
        SizePolicy::Fixed,
        220);

    workspace->addLayoutElement(
        inputArea,
        SizePolicy::Fixed,
        250);

    //==================================================
    // Job Items Section
    //==================================================

    auto itemsHeader =
        std::make_shared<Panel>();

    itemsHeader->setLayout(
        std::make_unique<HorizontalLayout>());

    itemsHeader->setStyle(
        PanelStyle::Card);

    itemsHeader->setBorderVisible(false);
    itemsHeader->setPadding(0);
    itemsHeader->setSpacing(10);

    workspace->addLayoutElement(
        itemsHeader,
        SizePolicy::Fixed,
        38);

    auto itemsTitle =
        std::make_shared<Label>();

    itemsTitle->setText(
        "Job Items");

    itemsTitle->setStyle(
        LabelStyle::Heading);

    itemsHeader->addLayoutElement(
        itemsTitle,
        SizePolicy::Fill,
        30);

    auto itemsHint =
        std::make_shared<Label>();

    itemsHint->setText(
        "Materials added to this job");

    itemsHint->setStyle(
        LabelStyle::Small);

    itemsHint->setTextTheme(
        TextTheme::DarkSecondary);

    itemsHeader->addLayoutElement(
        itemsHint,
        SizePolicy::Fixed,
        180);


    //==================================================
    // Job Item Grid
    //==================================================

    itemGrid =
        std::make_shared<DataGrid>();

    itemGrid->setSelectionChangedCallback(
        [this](int index)
        {
            loadItem(index);
        });

    itemGrid->addColumn(
        "Material",
        220);

    itemGrid->addColumn(
        "Variant",
        350);

    itemGrid->addColumn(
        "Size",
        180);

    itemGrid->addColumn(
        "Qty",
        100);

    workspace->addLayoutElement(
        itemGrid,
        SizePolicy::Fill);

    //--------------------------------------------------
    // Add Item
    //--------------------------------------------------

    addItemButton =
        std::make_shared<Button>();

    addItemButton->setText(
        "Add Item");


    //--------------------------------------------------
    // Remove Item
    //--------------------------------------------------

    removeItemButton =
        std::make_shared<Button>();

    removeItemButton->setText(
        "Remove Item");


    //--------------------------------------------------
    // Button Layout
    //--------------------------------------------------

    actionColumn->addLayoutElement(
        addItemButton,
        SizePolicy::Fixed,
        40);

    actionColumn->addLayoutElement(
        removeItemButton,
        SizePolicy::Fixed,
        40);

    //==================================================
    // Existing Add Item Logic
    //==================================================

    addItemButton->setOnClick(
        [this]()
        {
            JobItem item =
                createItem();

            std::string description =
                std::to_string(
                    currentJob.items.size()) +
                ". " +
                item.material.id +
                " | " +
                item.material
                .variants[item.variantIndex]
                .label +
                " | " +
                std::to_string(
                    (int)item.width) +
                " x " +
                std::to_string(
                    (int)item.height) +
                " | Qty " +
                std::to_string(
                    item.quantity);

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

                itemGrid->addRow(
                    {
                        item.material.id,
                        item.material.variants[item.variantIndex].label,
                        std::to_string((int)item.width) +
                            " x " +
                            std::to_string((int)item.height),
                        std::to_string(item.quantity)
                    },
                    activeJob
                    ? static_cast<int>(activeJob->items.size()) - 1
                    : static_cast<int>(currentJob.items.size()) - 1);
            }
            else
            {
                if (activeJob)
                {
                    activeJob->items[
                        editingIndex] = item;
                }

                itemGrid->updateRow(
                    editingIndex,
                    {
                        item.material.id,
                        item.material.variants[item.variantIndex].label,
                        std::to_string((int)item.width) +
                            " x " +
                            std::to_string((int)item.height),
                        std::to_string(item.quantity)
                    });

                editingIndex = -1;

                addItemButton->setText(
                    "Add Item");
            }

            width->setText("");
            height->setText("");
            quantity->setText("");

            width->setFocused(true);
        });

    //==================================================
    // Existing Remove Logic
    //==================================================

    removeItemButton->setOnClick(
        [this]()
        {
            int index =
                itemGrid->getSelectedRow();

            if (!activeJob)
                return;

            if (index < 0 ||
                index >= static_cast<int>(
                    activeJob->items.size()))
            {
                return;
            }

            itemGrid->removeRow(index);

            activeJob->items.erase(
                activeJob->items.begin() + index);
        });

    //==================================================
    // Calculate Button
    //==================================================

    calculateButton =
        std::make_shared<Button>();

    calculateButton->setText(
        "Calculate");

    calculateButton->setOnClick(
        [this]()
        {
            calculate();
        });

    //==================================================
    // Calculate Action Area
    //==================================================

    auto calculateRow =
        std::make_shared<Panel>();

    calculateRow->setLayout(
        std::make_unique<HorizontalLayout>());

    calculateRow->setBorderVisible(false);
    calculateRow->setPadding(0);
    calculateRow->setSpacing(0);

    calculateRow->addLayoutElement(
        calculateButton,
        SizePolicy::Fill);

    workspace->addLayoutElement(
        calculateRow,
        SizePolicy::Fixed,
        52);

    //==================================================
    // Format Visibility
    //==================================================

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

    itemGrid->clear();

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

    if (!activeJob)
        return;

    customerCompany->setText(
        activeJob->customer.company);

    customerContact->setText(
        activeJob->customer.contact);

    customerPhone->setText(
        activeJob->customer.phone);

    customerType->setText(
        activeJob->customer.accountType == AccountType::Cash
        ? "Cash Customer"
        : "Credit Customer");
}
