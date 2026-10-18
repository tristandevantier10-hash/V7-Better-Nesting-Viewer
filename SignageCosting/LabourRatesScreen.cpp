#include "LabourRatesScreen.h"
#include "Button.h"
#include "ComboBox.h"
#include "DataGrid.h"
#include "HorizontalLayout.h"
#include "Label.h"
#include "Metrics.h"
#include "PageHeader.h"
#include "Panel.h"
#include "PrimaryNavigation.h"
#include "TextBox.h"
#include "VerticalLayout.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <utility>

namespace
{
    const char* unitLabel(ProcessRateUnit unit)
    {
        switch (unit)
        {
        case ProcessRateUnit::PerSquareMetre: return "Per m2";
        case ProcessRateUnit::PerLinearMetre: return "Per linear m";
        case ProcessRateUnit::PerJob: return "Per job";
        default: return "Per item";
        }
    }
}

LabourRatesScreen::LabourRatesScreen()
{
    setLayout(std::make_unique<VerticalLayout>());
    auto header = std::make_shared<PageHeaderPanel>();
    header->setLayout(std::make_unique<HorizontalLayout>());
    configurePageHeader(header);
    addPageHeaderLogo(header);
    auto title = std::make_shared<Label>();
    title->setText("Labour Rates");
    title->setStyle(LabelStyle::Heading);
    configurePageHeaderTitleLabel(title);
    header->addLayoutElement(title, SizePolicy::Fill);
    auto back = std::make_shared<Button>();
    back->setText("Back");
    back->setOnClick([this]() { if (backCallback) backCallback(); });
    header->addLayoutElement(back, SizePolicy::Fixed, 100);
    addLayoutElement(header, SizePolicy::Fixed, Metrics::HeaderHeight);

    auto pageContent = std::make_shared<Panel>();
    pageContent->setLayout(std::make_unique<HorizontalLayout>());
    addLayoutElement(pageContent, SizePolicy::Fill);

    auto navigation = createPrimaryNavigationPanel(
        PrimaryNavigationRoute::LabourRates,
        [this](PrimaryNavigationRoute route)
        {
            if (navigationCallback)
                navigationCallback(route);
        });
    pageContent->addLayoutElement(
        navigation, SizePolicy::Fixed, Metrics::SidebarWidth);

    auto body = std::make_shared<Panel>();
    body->setLayout(std::make_unique<VerticalLayout>());
    body->setBorderVisible(false);
    body->setPadding(Metrics::SpaceXL);
    body->setSpacing(Metrics::SpaceM);
    pageContent->addLayoutElement(body, SizePolicy::Fill);

    auto intro = std::make_shared<Label>();
    intro->setText("Choose the processes your company offers, set their rates, or add a custom process.");
    intro->setStyle(LabelStyle::Small);
    intro->setTextTheme(TextTheme::DarkSecondary);
    body->addLayoutElement(intro, SizePolicy::Fixed, Metrics::LargeRow);

    grid = std::make_shared<DataGrid>();
    grid->addColumn("Process", 360);
    grid->addColumn("Rate", 150);
    grid->addColumn("Charged", 200);
    grid->addColumn("Available", 150);
    grid->setSelectionChangedCallback([this](int row) { loadSelected(row); });
    body->addLayoutElement(grid, SizePolicy::Fill);

    auto editor = std::make_shared<Panel>();
    editor->setLayout(std::make_unique<HorizontalLayout>());
    editor->setBorderVisible(false);
    editor->setSpacing(Metrics::SpaceS);
    name = std::make_shared<TextBox>();
    name->setPlaceholder("Process name");
    rate = std::make_shared<TextBox>();
    rate->setPlaceholder("Rate");
    unit = std::make_shared<ComboBox>();
    unit->addItem("Per m2"); unit->addItem("Per linear m");
    unit->addItem("Per item"); unit->addItem("Per job");
    editor->addLayoutElement(name, SizePolicy::Fill);
    editor->addLayoutElement(rate, SizePolicy::Fixed, 150);
    editor->addLayoutElement(unit, SizePolicy::Fixed, 190);
    body->addLayoutElement(editor, SizePolicy::Fixed, Metrics::ControlHeight);

    auto actions = std::make_shared<Panel>();
    actions->setLayout(std::make_unique<HorizontalLayout>());
    actions->setBorderVisible(false);
    actions->setSpacing(Metrics::SpaceS);
    saveButton = std::make_shared<Button>();
    saveButton->setText("Add process");
    saveButton->setOnClick([this]() { saveProcess(); });
    enabledButton = std::make_shared<Button>();
    enabledButton->setText("Toggle availability");
    enabledButton->setOnClick([this]() { toggleEnabled(); });
    actions->addLayoutElement(saveButton, SizePolicy::Fixed, 150);
    actions->addLayoutElement(enabledButton, SizePolicy::Fixed, 200);
    body->addLayoutElement(actions, SizePolicy::Fixed, Metrics::ControlHeight);
    refresh();
}

void LabourRatesScreen::refresh()
{
    grid->clear();
    const auto& processes = ProductionPricingDatabase::getProcesses();
    for (int i = 0; i < static_cast<int>(processes.size()); ++i)
    {
        const auto& process = processes[i];
        std::ostringstream rateText;
        rateText << "R " << std::fixed << std::setprecision(2) << process.rate;
        grid->addRow({ process.name, rateText.str(),
            unitLabel(process.unit), process.enabled ? "Enabled" : "Hidden" }, i);
    }
}

void LabourRatesScreen::loadSelected(int row)
{
    const auto& processes = ProductionPricingDatabase::getProcesses();
    if (row < 0 || row >= static_cast<int>(processes.size())) return;
    const auto& process = processes[row];
    editingId = process.id;
    name->setText(process.name);
    rate->setText(std::to_string(process.rate));
    unit->setSelectedIndex(static_cast<int>(process.unit));
    saveButton->setText("Update process");
    enabledButton->setText(process.enabled ? "Disable process" : "Enable process");
}

void LabourRatesScreen::saveProcess()
{
    if (name->getText().empty()) return;
    char* end = nullptr;
    const double value = std::strtod(rate->getText().c_str(), &end);
    if (!end || end == rate->getText().c_str() || *end != '\0' || value < 0.0) return;

    LabourProcess process;
    process.id = editingId;
    process.name = name->getText();
    process.rate = value;
    process.unit = static_cast<ProcessRateUnit>((std::max)(0, unit->getSelectedIndex()));
    if (editingId.empty())
    {
        process.id = "custom-";
        for (unsigned char c : process.name)
            if (std::isalnum(c)) process.id.push_back(static_cast<char>(std::tolower(c)));
            else if (process.id.back() != '-') process.id.push_back('-');
        while (!process.id.empty() && process.id.back() == '-') process.id.pop_back();
        process.id += "-" + std::to_string(SDL_GetTicks());
        process.enabled = true;
        if (!ProductionPricingDatabase::addProcess(process)) return;
    }
    else
    {
        auto existing = std::find_if(ProductionPricingDatabase::getProcesses().begin(),
            ProductionPricingDatabase::getProcesses().end(), [&](const LabourProcess& item)
            { return item.id == editingId; });
        if (existing != ProductionPricingDatabase::getProcesses().end())
            process.enabled = existing->enabled;
        if (!ProductionPricingDatabase::updateProcess(process)) return;
    }
    editingId.clear(); name->setText(""); rate->setText("");
    saveButton->setText("Add process");
    refresh();
}

void LabourRatesScreen::toggleEnabled()
{
    const auto& processes = ProductionPricingDatabase::getProcesses();
    const int row = grid->getSelectedRow();
    if (row < 0 || row >= static_cast<int>(processes.size())) return;
    LabourProcess changed = processes[row];
    changed.enabled = !changed.enabled;
    if (ProductionPricingDatabase::updateProcess(changed)) refresh();
}

void LabourRatesScreen::setBackCallback(std::function<void()> callback)
{
    backCallback = callback;
}

void LabourRatesScreen::setNavigationCallback(
    std::function<void(PrimaryNavigationRoute)> callback)
{
    navigationCallback = std::move(callback);
}
