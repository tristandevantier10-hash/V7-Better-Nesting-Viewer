#pragma once

#include "Panel.h"
#include "Label.h"
#include "PropertyRow.h"
#include "InvoiceViewModel.h"
#include "JobSummaryPanel.h"
#include <vector>

class Renderer;

class InvoicePanel : public Panel
{
public:

    InvoicePanel();

    void render(Renderer& renderer) override;

    void setViewModel(const InvoiceViewModel& vm);

private:

    std::shared_ptr<Label> title;

    std::shared_ptr<PropertyRow> materialCost;

    std::shared_ptr<PropertyRow> labourCost;

    std::shared_ptr<PropertyRow> productionCost;

    std::shared_ptr<PropertyRow> subtotal;

    std::shared_ptr<PropertyRow> markup;

    std::shared_ptr<PropertyRow> sellPrice;

    void buildStaticControls();

    void rebuildJobPanels(const InvoiceViewModel& vm);

    std::vector<std::shared_ptr<JobSummaryPanel>> jobPanels;
};
