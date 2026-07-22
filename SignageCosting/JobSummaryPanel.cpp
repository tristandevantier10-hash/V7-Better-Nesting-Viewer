#include "JobSummaryPanel.h"
#include "VerticalLayout.h"
#include "Renderer.h"

JobSummaryPanel::JobSummaryPanel()
{
    setLayout(std::make_unique<VerticalLayout>());

    title = std::make_shared<Label>();

    material = std::make_shared<PropertyRow>();
    variant = std::make_shared<PropertyRow>();
    quantity = std::make_shared<PropertyRow>();
    area = std::make_shared<PropertyRow>();
    rollWidth = std::make_shared<PropertyRow>();
    lengthUsed = std::make_shared<PropertyRow>();
    efficiency = std::make_shared<PropertyRow>();

    title->setText("Job Summary");

    material->setCaption("Material");
    variant->setCaption("Variant");
    quantity->setCaption("Quantity");
    area->setCaption("Area");
    rollWidth->setCaption("Roll Width");
    lengthUsed->setCaption("Length Used");
    efficiency->setCaption("Efficiency");

    addLayoutElement(title, SizePolicy::Fixed, 35);

    addLayoutElement(material, SizePolicy::Fixed, 26);
    addLayoutElement(variant, SizePolicy::Fixed, 26);
    addLayoutElement(quantity, SizePolicy::Fixed, 26);
    addLayoutElement(area, SizePolicy::Fixed, 26);
    addLayoutElement(rollWidth, SizePolicy::Fixed, 26);
    addLayoutElement(lengthUsed, SizePolicy::Fixed, 26);
    addLayoutElement(efficiency, SizePolicy::Fixed, 26);

    setExpanded(true);
}

void JobSummaryPanel::setMaterial(const std::string& value)
{
    material->setValue(value);
}

void JobSummaryPanel::setVariant(const std::string& value)
{
    variant->setValue(value);
}

void JobSummaryPanel::setQuantity(const std::string& value)
{
    quantity->setValue(value);
}

void JobSummaryPanel::setArea(const std::string& value)
{
    area->setValue(value);
}

void JobSummaryPanel::setRollWidth(const std::string& value)
{
    rollWidth->setValue(value);
}

void JobSummaryPanel::setLengthUsed(const std::string& value)
{
    lengthUsed->setValue(value);
}

void JobSummaryPanel::setEfficiency(const std::string& value)
{
    efficiency->setValue(value);
}

void JobSummaryPanel::render(Renderer& renderer)
{
    Panel::render(renderer);
}

void JobSummaryPanel::setTitle(const std::string& text)
{
    title->setText(text);
}

void JobSummaryPanel::setViewModel(const JobSummaryViewModel& vm)
{
    material->setValue(vm.material);
    variant->setValue(vm.variant);
    quantity->setValue(vm.quantity);
    area->setValue(vm.area);

    if (vm.isRoll)
    {
        rollWidth->setVisible(true);
        lengthUsed->setVisible(true);
        efficiency->setVisible(true);

        rollWidth->setCaption("Roll Width");
        rollWidth->setValue(vm.rollWidth);

        lengthUsed->setCaption("Length Used");
        lengthUsed->setValue(vm.lengthUsed);

        efficiency->setCaption("Efficiency");
        efficiency->setValue(vm.efficiency);
    }
    else
    {
        rollWidth->setVisible(true);
        lengthUsed->setVisible(true);
        efficiency->setVisible(false);

        rollWidth->setCaption("Sheet Size");
        rollWidth->setValue(vm.sheetSize);

        lengthUsed->setCaption("Sheets Used");
        lengthUsed->setValue(vm.sheetsUsed);
    }
}

void JobSummaryPanel::setExpanded(bool value)
{
    expanded = value;

    material->setVisible(expanded);
    variant->setVisible(expanded);
    quantity->setVisible(expanded);
    area->setVisible(expanded);
    rollWidth->setVisible(expanded);
    lengthUsed->setVisible(expanded);
    efficiency->setVisible(expanded);
}

bool JobSummaryPanel::isExpanded() const
{
    return expanded;
}