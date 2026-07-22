#pragma once

#include "Panel.h"
#include "Label.h"
#include "PropertyRow.h"
#include "JobSummaryViewModel.h"

class Renderer;

class JobSummaryPanel : public Panel
{
public:

    JobSummaryPanel();

    void setMaterial(const std::string&);
    void setVariant(const std::string&);
    void setQuantity(const std::string&);
    void setArea(const std::string&);
    void setRollWidth(const std::string&);
    void setLengthUsed(const std::string&);
    void setEfficiency(const std::string&);

    void setExpanded(bool value);
    bool isExpanded() const;

    void render(Renderer& renderer) override;
    void setTitle(const std::string& text);
    void setViewModel(const JobSummaryViewModel& vm);

private:

    std::shared_ptr<Label> title;
    std::shared_ptr<PropertyRow> material;
    std::shared_ptr<PropertyRow> variant;
    std::shared_ptr<PropertyRow> quantity;
    std::shared_ptr<PropertyRow> area;
    std::shared_ptr<PropertyRow> rollWidth;
    std::shared_ptr<PropertyRow> lengthUsed;
    std::shared_ptr<PropertyRow> efficiency;

    bool expanded = true;
};
