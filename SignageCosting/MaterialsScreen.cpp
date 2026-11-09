#include "MaterialsScreen.h"

#include "Button.h"
#include "Format.h"
#include "HorizontalLayout.h"
#include "MaterialDatabase.h"
#include "Metrics.h"
#include "PageHeader.h"
#include "Panel.h"
#include "PricingDatabase.h"
#include "VerticalLayout.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <sstream>
#include <utility>
#include <vector>

namespace
{
    std::string toLower(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(),
            [](unsigned char character)
            {
                return static_cast<char>(std::tolower(character));
            });
        return value;
    }

    std::string getFormats(const MaterialVariant& variant)
    {
        std::ostringstream result;
        bool hasFormat = false;

        if (!variant.roll_widths.empty())
        {
            result << "Rolls: ";
            for (std::size_t i = 0; i < variant.roll_widths.size(); ++i)
            {
                if (i > 0)
                    result << ", ";
                result << variant.roll_widths[i] << " mm";
            }
            hasFormat = true;
        }

        if (!variant.sheet_formats.empty())
        {
            if (hasFormat)
                result << " | ";
            result << "Sheets: ";
            for (std::size_t i = 0; i < variant.sheet_formats.size(); ++i)
            {
                if (i > 0)
                    result << ", ";
                result << variant.sheet_formats[i].width << " x "
                    << variant.sheet_formats[i].height << " mm";
            }
            hasFormat = true;
        }

        return hasFormat ? result.str() : "—";
    }
}

MaterialsScreen::MaterialsScreen(std::shared_ptr<NavigationPanel> sharedNavigation)
{
    setPadding(0);
    setSpacing(0);


    auto content = std::make_shared<Panel>();
    content->setLayout(std::make_unique<VerticalLayout>());
    content->setPadding(0);
    content->setSpacing(0);

    auto header = std::make_shared<DashboardBannerPanel>();
    header->setLayout(std::make_unique<HorizontalLayout>());
    configurePageHeader(header);

    auto titlePanel = std::make_shared<PageHeaderContentPanel>();
    titlePanel->setLayout(std::make_unique<VerticalLayout>());
    configurePageHeaderTitle(titlePanel);

    auto title = std::make_shared<Label>();
    title->setText("MATERIALS");
    title->setStyle(LabelStyle::Heading);
    configurePageHeaderTitleLabel(title);
    titlePanel->addLayoutElement(title, SizePolicy::Fixed, 34);

    summaryLabel = std::make_shared<Label>();
    summaryLabel->setStyle(LabelStyle::Small);
    configurePageHeaderSubtitleLabel(summaryLabel);
    auto description = std::make_shared<Label>();
    description->setText("Manage your material database.");
    configurePageHeaderSubtitleLabel(description);
    titlePanel->addLayoutElement(description, SizePolicy::Fixed);
    header->addLayoutElement(titlePanel, SizePolicy::Fill);
    header->addLayoutElement(summaryLabel, SizePolicy::Fixed, 190);

    auto backButton = std::make_shared<Button>();
    backButton->setText("Back");
    backButton->setOnClick([this]()
    {
        if (backCallback)
            backCallback();
    });
    addPageHeaderActions(header, nullptr, backButton);
    auto workspaceBody = addWorkspaceLayout(*this, header);
    workspaceBody->addLayoutElement(sharedNavigation, SizePolicy::Fixed, Metrics::SidebarWidth);
    workspaceBody->addLayoutElement(content, SizePolicy::Fill);

    auto body = std::make_shared<Panel>();
    body->setLayout(std::make_unique<VerticalLayout>());
    body->setStyle(PanelStyle::Workspace);
    body->setPadding(Metrics::WorkspaceGap);
    body->setSpacing(Metrics::WorkspaceGap);
    content->addLayoutElement(body, SizePolicy::Fill);

    searchBox = std::make_shared<TextBox>();
    searchBox->setCompactText(true);
    searchBox->setPlaceholder("Search materials, variants, category, type or use...");
    searchBox->setTextChangedCallback([this](const std::string& query)
    {
        filterMaterials(query);
    });
    body->addLayoutElement(searchBox, SizePolicy::Fixed, Metrics::CompactControlHeight);

    materialsGrid = std::make_shared<DataGrid>();
    materialsGrid->addColumn("Material", 145);
    materialsGrid->addColumn("Variant", 200);
    materialsGrid->addColumn("Type", 110);
    materialsGrid->addColumn("Use", 130);
    materialsGrid->addColumn("Formats", 260);
    materialsGrid->addColumn("Cost / m²", 100);
    materialsGrid->addColumn("Life", 65);
    materialsGrid->setRowHeight(Metrics::NormalRow);
    materialsGrid->setTextStyle(LabelStyle::Small);
    materialsGrid->setEmptyMessage("No materials match your search.");
    materialsGrid->setResizableColumns(true);
    materialsGrid->setShowColumnHeader(false);
    materialsGrid->setOuterBorderVisible(false);
    materialsGrid->setHeaderContentGap(0);
    body->addLayoutElement(materialsGrid, SizePolicy::Fill);

    refreshMaterials();
}

void MaterialsScreen::refreshMaterials()
{
    filterMaterials(searchBox ? searchBox->getText() : std::string{});
}

void MaterialsScreen::setBackCallback(std::function<void()> callback)
{
    backCallback = std::move(callback);
}

void MaterialsScreen::filterMaterials(const std::string& query)
{
    if (!materialsGrid)
        return;

    materialsGrid->clear();
    std::vector<std::string> materialIds = MaterialDatabase::getBaseMaterials();
    std::sort(materialIds.begin(), materialIds.end(),
        [](const std::string& left, const std::string& right)
        {
            const Material leftMaterial = MaterialDatabase::get(left);
            const Material rightMaterial = MaterialDatabase::get(right);
            const std::string leftName = leftMaterial.name.empty() ? left : leftMaterial.name;
            const std::string rightName = rightMaterial.name.empty() ? right : rightMaterial.name;
            return toLower(leftName) < toLower(rightName);
        });

    const std::string normalizedQuery = toLower(query);
    int visibleMaterials = 0;
    int visibleVariants = 0;
    std::map<std::string, std::vector<std::vector<std::string>>> rowsByCategory;

    for (const std::string& id : materialIds)
    {
        const Material material = MaterialDatabase::get(id);
        const std::string materialName = material.name.empty() ? material.id : material.name;
        bool materialMatched = false;
        const std::string category = material.category.empty()
            ? "Uncategorized"
            : material.category;

        for (const MaterialVariant& variant : material.variants)
        {
            const std::string formats = getFormats(variant);
            const std::vector<std::string> row =
            {
                materialName,
                variant.label,
                variant.type,
                variant.usage,
                formats,
                Format::money(PricingDatabase::getMaterialCost(material.id, variant.label)),
                variant.duration_years > 0 ? std::to_string(variant.duration_years) + " yr" : "—"
            };

            std::string searchableText;
            for (const std::string& value : row)
            {
                searchableText += value;
                searchableText.push_back(' ');
            }
            searchableText += material.id;
            searchableText += " ";
            searchableText += category;

            if (!normalizedQuery.empty() &&
                toLower(searchableText).find(normalizedQuery) == std::string::npos)
            {
                continue;
            }

            rowsByCategory[category].push_back(row);
            materialMatched = true;
            ++visibleVariants;
        }

        if (materialMatched)
            ++visibleMaterials;
    }

    for (const auto& [category, rows] : rowsByCategory)
    {
        materialsGrid->addGroupHeader("Category: " + category);
        for (const auto& row : rows)
            materialsGrid->addRow(row);
    }

    if (summaryLabel)
    {
        summaryLabel->setText(
            std::to_string(visibleMaterials) + " materials  ·  " +
            std::to_string(visibleVariants) + " variants");
    }
}
