#include "InvoicePanel.h"
#include "VerticalLayout.h"
#include "Renderer.h"
#include "iostream"
#include <windows.h>

InvoicePanel::InvoicePanel()
{
    setLayout(std::make_unique<VerticalLayout>());

    setScrollable(true);

    setPosition(10, 10);
    setSize(300, 300);

    title = std::make_shared<Label>();

    materialCost = std::make_shared<PropertyRow>();
    labourCost = std::make_shared<PropertyRow>();
    productionCost = std::make_shared<PropertyRow>();
    subtotal = std::make_shared<PropertyRow>();
    markup = std::make_shared<PropertyRow>();
    sellPrice = std::make_shared<PropertyRow>();

    buildStaticControls();
}

void InvoicePanel::render(Renderer& renderer)
{
    Panel::render(renderer);
}

void InvoicePanel::buildStaticControls()
{
    title->setText("Invoice");

    materialCost->setCaption("Material");

    labourCost->setCaption("Labour");

    productionCost->setCaption("Production");

    subtotal->setCaption("Subtotal");

    markup->setCaption("Markup");

    sellPrice->setCaption("SELL PRICE");

    addLayoutElement(title, SizePolicy::Fixed, 35);

    addLayoutElement(materialCost, SizePolicy::Fixed, 26);
    addLayoutElement(labourCost, SizePolicy::Fixed, 26);
    addLayoutElement(productionCost, SizePolicy::Fixed, 26);
    addLayoutElement(subtotal, SizePolicy::Fixed, 26);
    addLayoutElement(markup, SizePolicy::Fixed, 26);
    addLayoutElement(sellPrice, SizePolicy::Fixed, 35);
}

void InvoicePanel::setViewModel(const InvoiceViewModel& vm)
{
    materialCost->setValue(vm.materialCost);
    labourCost->setValue(vm.labourCost);
    productionCost->setValue(vm.productionCost);
    subtotal->setValue(vm.subtotal);
    markup->setValue(vm.markup);
    sellPrice->setValue(vm.sellPrice);

    std::cout << "\n===== InvoicePanel::setViewModel =====\n";
    std::cout << vm.materialCost << std::endl;
    std::cout << vm.labourCost << std::endl;
    std::cout << vm.sellPrice << std::endl;

    rebuildJobPanels(vm);
}

void InvoicePanel::rebuildJobPanels(const InvoiceViewModel& vm)
{
    jobPanels.clear();

    clearChildren();

    buildStaticControls();

    int jobNumber = 1;

    for (const auto& job : vm.jobs)
    {
        auto panel = std::make_shared<JobSummaryPanel>();

        panel->setTitle(
            "JOB "
            + std::to_string(jobNumber)
            + " - "
            + job.material);

        panel->setViewModel(job);

        jobPanels.push_back(panel);

        addLayoutElement(panel, SizePolicy::Fixed, 220);

        jobNumber++;
    }
}