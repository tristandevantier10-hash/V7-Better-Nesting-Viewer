#include "NestingScreen.h"
#include "NestingWorkspace.h"
#include "Button.h"
#include "CardPanel.h"
#include "HorizontalLayout.h"
#include "VerticalLayout.h"
#include "PageHeader.h"
#include "Metrics.h"
#include "NestingCore.h"
#include "NestingLayoutSearch.h"
#include "NestingOutlineEditor.h"
#include "ComboBox.h"
#include "DxfImport.h"
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <chrono>
#include <algorithm>

namespace
{
    // Keep mouse input aligned with rows rendered at a scroll offset.
    class SizeListPanel : public Panel
    {
    public:
        int rowHeight = Metrics::ControlHeight;
        void render(Renderer& renderer) override
        {
            clampScroll();
            Panel::render(renderer);
        }

        void update(const SDL_Event& event) override
        {
            performLayout();
            SDL_Event translated = event;
            SDL_Point mouse{};
            SDL_GetMouseState(&mouse.x, &mouse.y);
            if (event.type == SDL_MOUSEMOTION)
                mouse = { event.motion.x, event.motion.y };
            else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
                mouse = { event.button.x, event.button.y };
            const bool inside = SDL_PointInRect(&mouse, &bounds);
            if (event.type == SDL_MOUSEWHEEL)
            {
                if (inside) scrollY -= event.wheel.y * scrollSpeed;
                clampScroll();
                return;
            }
            clampScroll();
            if (event.type == SDL_MOUSEMOTION)
            {
                translated.motion.x = inside ? mouse.x : -100000;
                translated.motion.y = inside ? mouse.y + scrollY : -100000;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
            {
                translated.button.x = inside ? mouse.x : -100000;
                translated.button.y = inside ? mouse.y + scrollY : -100000;
                SDL_Event hover{};
                hover.type = SDL_MOUSEMOTION;
                hover.motion.x = translated.button.x;
                hover.motion.y = translated.button.y;
                for (auto& child : children)
                    if (child->isVisible()) child->update(hover);
            }
            for (auto& child : children)
                if (child->isVisible()) child->update(translated);
        }

    private:
        void clampScroll()
        {
            int count = 0;
            for (const auto& child : children) if (child->isVisible()) ++count;
            const int height = count * (rowHeight + Metrics::SpaceS) - Metrics::SpaceS;
            scrollY = std::clamp(scrollY, 0, std::max(0, height - getHeight()));
        }
    };

    double readDimension(const std::shared_ptr<TextBox>& field)
    {
        const auto& text = field->getText();
        size_t consumed = 0;
        const double value = std::stod(text, &consumed);
        if (text.find_first_not_of(" \t", consumed) != std::string::npos ||
            !std::isfinite(value) || value <= 0.0 || value > 1000000.0)
            throw std::invalid_argument("dimension");
        return value;
    }
}

NestingScreen::NestingScreen(std::shared_ptr<NavigationPanel> sharedNavigation)
    : navigation(std::move(sharedNavigation))
{
    setLayout(std::make_unique<HorizontalLayout>());
    setPadding(0);
    setSpacing(0);
    addLayoutElement(navigation, SizePolicy::Fixed, Metrics::SidebarWidth);

    auto content = std::make_shared<Panel>();
    content->setLayout(std::make_unique<VerticalLayout>());
    content->setPadding(0);
    content->setSpacing(0);
    addLayoutElement(content, SizePolicy::Fill);
    auto header = std::make_shared<DashboardBannerPanel>();
    header->setLayout(std::make_unique<HorizontalLayout>());
    configurePageHeader(header);
    auto title = std::make_shared<Label>();
    title->setText("Nesting");
    title->setStyle(LabelStyle::Heading);
    configurePageHeaderTitleLabel(title);
    header->addLayoutElement(title, SizePolicy::Fill);
    auto geometryLabel=std::make_shared<Label>();
    geometryLabel->setText("Geometry");
    geometryLabel->setStyle(LabelStyle::Small);
    header->addLayoutElement(geometryLabel,SizePolicy::Fixed,80);
    geometrySelector=std::make_shared<ComboBox>();
    geometrySelector->addItem("Rectangles");
    geometrySelector->addItem("Custom shapes");
    geometrySelector->setSelectedIndex(0);
    auto comboHolder=[](const std::shared_ptr<ComboBox>& combo)
    {
        auto holder=std::make_shared<PageHeaderContentPanel>();
        auto layout=std::make_unique<VerticalLayout>();
        layout->setCenterContent(true);
        holder->setLayout(std::move(layout));
        holder->setPadding(0); holder->setBorderVisible(false);
        holder->addLayoutElement(combo,SizePolicy::Fixed,Metrics::ControlHeight);
        return holder;
    };
    header->addLayoutElement(comboHolder(geometrySelector),SizePolicy::Fixed,190);
    rotationSelector=std::make_shared<ComboBox>();
    rotationSelector->addItem("90° rotation steps");
    rotationSelector->addItem("Keep orientation");
    rotationSelector->setSelectedIndex(0);
    rotationSelector->setVisible(false);
    rotationSelector->setSelectionChangedCallback([this](int) { invalidate(); });
    rotationControls=comboHolder(rotationSelector);
    rotationControls->setVisible(false);
    header->addLayoutElement(rotationControls,SizePolicy::Fixed,185);
    geometrySelector->setSelectionChangedCallback([this](int) { refreshGeometryMode(); });
    content->addLayoutElement(header, SizePolicy::Fixed, 132);

    workspace = std::make_shared<NestingWorkspacePanel>();
    workspace->arrange = [this]() { arrangeWorkspace(); };
    workspace->paint = [this](Renderer& renderer) { paintWorkspace(renderer); };
    workspace->setPadding(0);
    workspace->setBorderVisible(false);
    content->addLayoutElement(workspace, SizePolicy::Fill);

    auto field = [this](const char* value, std::shared_ptr<TextBox>& target)
    {
        target = std::make_shared<TextBox>();
        target->setText(value);
        target->setTextChangedCallback([this](const std::string&) { invalidate(); });
        workspace->add(target);
    };
    field("2440", sheetWidth);
    field("1220", sheetHeight);
    sizes = std::make_shared<SizeListPanel>();
    sizes->setLayout(std::make_unique<VerticalLayout>());
    sizes->setPadding(0);
    sizes->setSpacing(Metrics::SpaceS);
    sizes->setBorderVisible(false);
    sizes->setScrollable(true);
    workspace->add(sizes);
    addSize();
    addButton = std::make_shared<NestingPageButton>("+  Add size");
    addButton->setOnClick([this]() { addSize(); });
    workspace->add(addButton);
    importButton=std::make_shared<NestingPageButton>("Import DXF");
    importButton->setOnClick([this]() { chooseDxf(); });
    workspace->add(importButton);
    calculateButton = std::make_shared<NestingPageButton>("Calculate nesting", true);
    calculateButton->setOnClick([this]() { calculate(); });
    workspace->add(calculateButton);
    summary = std::make_shared<Label>();
    summary->setText("Add sizes and calculate a combined layout.");
    details = std::make_shared<Label>();
    preview = std::make_shared<NestingPreviewPanel>();
    preview->status = summary->getText();
    workspace->add(preview);
    outlineEditor=std::make_shared<NestingOutlineEditor>();
    outlineEditor->setVisible(false);
    outlineEditor->cancelled=[this]() { outlineEditor->setVisible(false); };
}

void NestingScreen::arrangeWorkspace()
{
    const auto bounds = workspace->getBounds();
    const int gap = 10;
    const int leftWidth = std::clamp(static_cast<int>(bounds.w * 0.32), 280, 380);
    setupBounds = { bounds.x + gap, bounds.y + gap, leftWidth - gap, bounds.h - 2 * gap };
    const int rightX = setupBounds.x + setupBounds.w + gap;
    metricsBounds = { rightX, bounds.y + gap, bounds.x + bounds.w - gap - rightX, 78 };
    auto position = [](const std::shared_ptr<UIElement>& item, const SDL_Rect& rect)
    {
        item->setPosition(rect.x, rect.y);
        item->setSize(rect.w, rect.h);
    };
    position(sheetWidth, { setupBounds.x + 16, setupBounds.y + 94, setupBounds.w - 32, 34 });
    position(sheetHeight, { setupBounds.x + 16, setupBounds.y + 162, setupBounds.w - 32, 34 });
    const int rowTop = setupBounds.y + 304;
    const int available = std::max(40, setupBounds.h - 304 - 164);
    const int rowsHeight = std::min(static_cast<int>(rows.size()) * (sizeRowHeight()+Metrics::SpaceS) - Metrics::SpaceS, available);
    std::static_pointer_cast<SizeListPanel>(sizes)->rowHeight=sizeRowHeight();
    tableBounds = { setupBounds.x + 16, setupBounds.y + 274, setupBounds.w - 32, rowsHeight + 38 };
    position(sizes, { tableBounds.x + 1, rowTop, tableBounds.w - 2, rowsHeight });
    const int half=(setupBounds.w-38)/2;
    position(addButton, { setupBounds.x + 16, rowTop + rowsHeight + 20, half, 38 });
    position(importButton, { setupBounds.x + 22 + half, rowTop + rowsHeight + 20, setupBounds.w-38-half, 38 });
    position(calculateButton, { setupBounds.x + 16, setupBounds.y + setupBounds.h - 66, setupBounds.w - 32, 48 });
    position(preview, { rightX, metricsBounds.y + metricsBounds.h + 12, metricsBounds.w,
        setupBounds.h - metricsBounds.h - 12 });
    preview->status = summary->getText();
}

void NestingScreen::paintWorkspace(Renderer& renderer)
{
    using namespace NestingStyle;
    renderer.fillRect(workspace->getBounds(), { 246, 248, 250, 255 });
    card(renderer, setupBounds);
    card(renderer, metricsBounds);
    const int x = setupBounds.x + 16;
    const int y = setupBounds.y;
    text(renderer, "Sheet setup", x, y + 16, LabelStyle::Heading);
    text(renderer, "All dimensions in mm", x, y + 49, LabelStyle::Small, muted);
    text(renderer, "Sheet width", x, y + 74);
    text(renderer, "Sheet height", x, y + 142);
    renderer.drawLine(x, y + 226, setupBounds.x + setupBounds.w - 16, y + 226, border);
    text(renderer, customMode() ? "Pieces & outlines" : "Piece sizes", x, y + 242, LabelStyle::Normal);
    size_t total = 0;
    for (const auto& row : rows)
    {
        try
        {
            const double value = readDimension(row->quantity);
            if (value <= 2000 && std::floor(value) == value) total += static_cast<size_t>(value);
        }
        catch (const std::exception&) {}
    }
    const std::string totals = std::to_string(rows.size()) + " sizes · " + std::to_string(total) + " pieces";
    int tw = 0, th = 0;
    TTF_SizeUTF8(renderer.getFontManager().getSmallFont(), totals.c_str(), &tw, &th);
    text(renderer, totals, setupBounds.x + setupBounds.w - 16 - tw, y + 247, LabelStyle::Small, muted);
    renderer.fillRoundedRect(tableBounds, { 252, 253, 254, 255 }, 5);
    renderer.drawRoundedRect(tableBounds, border, 5);
    const int colourWidth = 55, rowGap = 6;
    const int fieldWidth = (tableBounds.w - 2 - 48 - 28 - rowGap * 4) / 3;
    text(renderer, "Colour", tableBounds.x + 7, tableBounds.y + 8, LabelStyle::Small, muted);
    for (int column = 0; column < 3; ++column)
        text(renderer, std::array<const char*, 3>{ "Width", "Height", "Qty" }[column],
            tableBounds.x + colourWidth + column * (fieldWidth + rowGap), tableBounds.y + 8,
            LabelStyle::Small, muted);
    renderer.drawLine(tableBounds.x, tableBounds.y + 29, tableBounds.x + tableBounds.w,
        tableBounds.y + 29, border);
    text(renderer, customMode() ? "Edit each outline · No cutting gap" : "Rotation allowed · No cutting gap", x,
        setupBounds.y + setupBounds.h - 99, LabelStyle::Small, muted);
    for (int index = 0; index < 4; ++index)
    {
        SDL_Rect cell{ metricsBounds.x + index * metricsBounds.w / 4, metricsBounds.y,
            metricsBounds.w / 4, metricsBounds.h };
        if (index > 0) renderer.drawLine(cell.x, cell.y + 16, cell.x, cell.y + cell.h - 16, border);
        TTF_SizeUTF8(renderer.getFontManager().getHeadingFont(), metricValues[index].c_str(), &tw, &th);
        renderer.drawText(metricValues[index], cell.x + (cell.w - tw) / 2, cell.y + 12,
            renderer.getFontManager().getHeadingFont(), ink);
        const std::string label = std::array<const char*, 4>{ "Sheets", "Pieces", "Utilisation", "Waste" }[index];
        TTF_SizeUTF8(renderer.getFontManager().getSmallFont(), label.c_str(), &tw, &th);
        text(renderer, label, cell.x + (cell.w - tw) / 2, cell.y + 47, LabelStyle::Small, muted);
    }
}

void NestingScreen::addSize()
{
    if (rows.size() >= 50)
    {
        if (summary) summary->setText("Use up to 50 sizes per layout.");
        return;
    }
    auto row = std::make_shared<SizeRow>();
    row->panel = std::make_shared<Panel>();
    row->panel->setLayout(std::make_unique<VerticalLayout>());
    row->panel->setPadding(0);
    row->panel->setBorderVisible(false);
    row->panel->setSpacing(Metrics::SpaceS);
    auto fields=std::make_shared<Panel>();
    fields->setLayout(std::make_unique<HorizontalLayout>());
    fields->setBorderVisible(false);
    fields->setPadding(0);
    fields->setSpacing(Metrics::SpaceS);
    row->panel->addLayoutElement(fields,SizePolicy::Fixed,Metrics::ControlHeight);
    row->swatch = std::make_shared<NestingSizeSwatch>();
    row->swatch->index = rows.size();
    fields->addLayoutElement(row->swatch, SizePolicy::Fixed, 48);
    auto field = [this, fields](const char* value, std::shared_ptr<TextBox>& target)
    {
        target = std::make_shared<TextBox>();
        target->setText(value);
        target->setTextChangedCallback([this](const std::string&) { invalidate(); });
        fields->addLayoutElement(target, SizePolicy::Fill);
    };
    field("600", row->width);
    field("400", row->height);
    field("10", row->quantity);
    auto remove = std::make_shared<NestingPageButton>("×");
    // A weak reference avoids a callback ownership cycle with the row panel.
    std::weak_ptr<SizeRow> weakRow = row;
    remove->setOnClick([this, weakRow]()
    {
        auto selected = weakRow.lock();
        if (!selected || rows.size() <= 1) return;
        selected->panel->setVisible(false);
        rows.erase(std::remove(rows.begin(), rows.end(), selected), rows.end());
        rebuildRows = true;
        invalidate();
    });
    fields->addLayoutElement(remove, SizePolicy::Fixed, 28);
    row->shapeButton=std::make_shared<NestingPageButton>("Rectangle · Edit outline");
    row->shapeButton->setVisible(customMode());
    row->shapeButton->setOnClick([this,weakRow]() { if (auto selected=weakRow.lock()) editOutline(selected); });
    row->width->setTextChangedCallback([this,weakRow](const std::string& value)
    {
        if (auto selected=weakRow.lock())
        {
            selected->pristine=false;
            if (customMode() && selected->shapeName=="Circle") selected->height->setText(value);
        }
        invalidate();
    });
    row->height->setTextChangedCallback([this,weakRow](const std::string& value)
    {
        if (auto selected=weakRow.lock())
        {
            selected->pristine=false;
            if (customMode() && selected->shapeName=="Circle") selected->width->setText(value);
        }
        invalidate();
    });
    row->quantity->setTextChangedCallback([this,weakRow](const std::string&)
    { if (auto selected=weakRow.lock()) selected->pristine=false; invalidate(); });
    row->panel->addLayoutElement(row->shapeButton,SizePolicy::Fixed,30);
    sizes->addLayoutElement(row->panel, SizePolicy::Fixed, sizeRowHeight());
    rows.push_back(row);
    invalidate();
}

bool NestingScreen::customMode() const { return geometrySelector->getSelectedIndex()==1; }
int NestingScreen::sizeRowHeight() const { return customMode() ? Metrics::ControlHeight+Metrics::SpaceS+30 : Metrics::ControlHeight; }

void NestingScreen::refreshGeometryMode()
{
    rotationControls->setVisible(customMode());
    rotationSelector->setVisible(customMode());
    for (auto& row:rows)
    {
        row->shapeButton->setVisible(customMode());
        if (customMode() && row->shapeName=="Circle") row->height->setText(row->width->getText());
    }
    rebuildRows=true;
    invalidate();
}

void NestingScreen::chooseDxf()
{
    std::string error;
    const auto path=DxfImport::chooseFile(error);
    if (!error.empty()) summary->setText(error);
    if (!path.empty()) importDxfFile(path);
}

void NestingScreen::importDxfFile(const std::filesystem::path& path)
{
    auto result=DxfImport::load(path);
    if (result.valid() && result.unitless)
    {
        const SDL_MessageBoxButtonData buttons[]{
            {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT,1,"Millimetres"},
            {0,2,"Inches"},{SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT,0,"Cancel"}};
        const SDL_MessageBoxData dialog{SDL_MESSAGEBOX_INFORMATION,nullptr,"DXF drawing units",
            "This DXF does not declare drawing units. How should its dimensions be interpreted?",3,buttons,nullptr};
        int selected=0;
        if (SDL_ShowMessageBox(&dialog,&selected)!=0 || selected==0) return;
        if (selected==2) result=DxfImport::load(path,25.4);
    }
    const bool replaceDefault=rows.size()==1 && rows.front()->pristine;
    const size_t retained=replaceDefault ? 0 : rows.size();
    if (result.valid() && retained+result.outlines.size()>50)
        result.error="The import would exceed 50 size rows. Remove some rows or import a smaller file.";
    if (!result.valid())
    {
        summary->setText("DXF import failed. Existing pieces were kept.");
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"DXF import failed",result.error.c_str(),nullptr);
        return;
    }
    // Parse and validate the whole file before changing any existing row.
    if (replaceDefault) { rows.clear(); sizes->clearChildren(); }
    geometrySelector->setSelectedIndex(1);
    refreshGeometryMode();
    for (const auto& source:result.outlines)
    {
        addSize();
        auto& row=rows.back();
        const auto box=NestingGeometry::bounds(source.polygon);
        const double width=box.right-box.left,height=box.bottom-box.top;
        row->outline=source.polygon;
        for (auto& point:row->outline.vertices)
        { point.x=(point.x-box.left)/width; point.y=(point.y-box.top)/height; }
        std::ostringstream w,h; w<<std::setprecision(10)<<width; h<<std::setprecision(10)<<height;
        row->width->setText(w.str()); row->height->setText(h.str()); row->quantity->setText("1");
        row->shapeName=source.circle ? "Circle" : source.name;
        row->shapeButton->setCaption(row->shapeName+" · Edit outline");
        row->pristine=false;
    }
    rebuildRows=true;
    invalidate();
    summary->setText("Imported "+std::to_string(result.outlines.size())+" outlines. Review sizes and quantities, then Calculate.");
    if (!result.warnings.empty()) SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,"DXF import",result.warnings.front().c_str(),nullptr);
}

void NestingScreen::editOutline(const std::shared_ptr<SizeRow>& row)
{
    try
    {
        const double width=readDimension(row->width),height=readDimension(row->height);
        std::weak_ptr<SizeRow> target=row;
        outlineEditor->accepted=[this,target](const NestingGeometry::Polygon& polygon,const std::string& name)
        {
            if (auto selected=target.lock())
            {
                selected->outline=polygon;
                selected->shapeName=name;
                selected->pristine=false;
                if (name=="Circle") selected->height->setText(selected->width->getText());
                selected->shapeButton->setCaption(name+" · Edit outline");
                invalidate();
            }
            outlineEditor->setVisible(false);
        };
        outlineEditor->begin(row->outline,row->shapeName,width,height);
    }
    catch (const std::exception&) { summary->setText("Enter a positive width and height before editing the outline."); }
}

void NestingScreen::update(const SDL_Event& event)
{
    if (event.type==SDL_DROPFILE)
    {
        if (!event.drop.file) return;
        const std::filesystem::path path=std::filesystem::u8path(event.drop.file);
        SDL_free(event.drop.file);
        if (!outlineEditor->isVisible()) importDxfFile(path);
        return;
    }
    if (outlineEditor->isVisible())
    {
        outlineEditor->setPosition(0,0); outlineEditor->setSize(getWidth(),getHeight());
        outlineEditor->update(event);
    }
    else Screen::update(event);
}

void NestingScreen::render(Renderer& renderer)
{
    Screen::render(renderer);
    if (outlineEditor->isVisible())
    {
        outlineEditor->setPosition(0,0); outlineEditor->setSize(getWidth(),getHeight());
        outlineEditor->render(renderer);
    }
}

void NestingScreen::onActivated()
{
    navigation->selectItem(navigation->findItemByText("Nesting"));
}

void NestingScreen::invalidate()
{
    ++revision;
    if (!preview) return;
    preview->setSheets({});
    metricValues = { "—", "—", "—", "—" };
    summary->setText("Inputs changed. Calculate to update the layout.");
    details->setText("");
}

void NestingScreen::calculate()
{
    if (calculation.valid())
    {
        summary->setText("Nesting is running. Please wait before calculating again.");
        return;
    }
    preview->setSheets({});
    details->setText("");
    try
    {
        const double sw = readDimension(sheetWidth), sh = readDimension(sheetHeight);
        std::vector<NestingGeometry::Part> items;
        double usedArea = 0.0;
        int requested = 0;
        for (size_t index = 0; index < rows.size(); ++index)
        {
            const auto& row = rows[index];
            const double pw = readDimension(row->width), ph = readDimension(row->height);
            const double count = readDimension(row->quantity);
            if (count > 2000 || std::floor(count) != count || requested + count > 2000)
            {
                summary->setText("Use whole quantities, up to 2,000 pieces total.");
                return;
            }
            if (!customMode() && !((pw <= sw && ph <= sh) || (ph <= sw && pw <= sh)))
            {
                summary->setText("Size " + std::to_string(index + 1) + " does not fit the sheet.");
                details->setText("Check its dimensions; both orientations were considered.");
                return;
            }
            auto outline=customMode() ? row->outline : NestingGeometry::rectangle(1,1);
            for (auto& point:outline.vertices) { point.x*=pw; point.y*=ph; }
            const auto rotations=customMode()
                ? (rotationSelector->getSelectedIndex()==0 ? std::vector<double>{0,90,180,270} : std::vector<double>{0})
                : std::vector<double>{0,90};
            items.push_back({row->partId,outline,static_cast<int>(count),rotations,customMode() ? row->shapeName : "Rectangle"});
            requested += static_cast<int>(count);
            usedArea += NestingGeometry::area(outline) * count;
        }
        preview->setGeometryParts(items);
        calculationRevision = revision;
        calculationArea = usedArea;
        calculationSheetArea = sw * sh;
        calculationSizeCount = rows.size();
        calculation = std::async(std::launch::async, [items, sw, sh]()
        {
            return NestingService::calculate(items,{sw,sh});
        });
        summary->setText("Calculating the combined layout...");
    }
    catch (const std::exception&)
    {
        summary->setText("Enter positive numbers for all dimensions.");
        details->setText("Use millimetres and a whole quantity from 1 to 2,000.");
    }
}

void NestingScreen::tick(float deltaTime)
{
    if (rebuildRows)
    {
        sizes->clearChildren();
        for (size_t index = 0; index < rows.size(); ++index)
        {
            rows[index]->swatch->index = index;
            sizes->addLayoutElement(rows[index]->panel, SizePolicy::Fixed, sizeRowHeight());
        }
        rebuildRows = false;
    }
    Screen::tick(deltaTime);
    if (!calculation.valid() ||
        calculation.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
        return;
    try
    {
        auto calculated = calculation.get();
        auto& best = calculated.layout;
        if (calculationRevision != revision) return;
        if (!best.isValid() || best.sheets.empty())
        {
            summary->setText(best.validationErrors.empty() ? "No valid layout found. Check the dimensions." : best.validationErrors.front());
            return;
        }
        size_t count = 0;
        for (const auto& sheet : best.sheets) count += sheet.placed.size();
        const double sheetArea = best.sheets.size() * calculationSheetArea;
        summary->setText(std::to_string(count) + " pieces / " + std::to_string(calculationSizeCount) +
            " sizes / " + std::to_string(best.sheets.size()) + " sheets");
        std::ostringstream info;
        info << std::fixed << std::setprecision(1)
            << "Used: " << calculationArea / sheetArea * 100.0 << "%  |  Waste: "
            << (1.0 - calculationArea / sheetArea) * 100.0 << "%  |  Best layout found";
        details->setText(info.str());
        std::ostringstream used, waste;
        used << std::fixed << std::setprecision(1) << calculationArea / sheetArea * 100.0 << "%";
        waste << std::fixed << std::setprecision(1) << (1.0 - calculationArea / sheetArea) * 100.0 << "%";
        metricValues = { std::to_string(best.sheets.size()), std::to_string(count), used.str(), waste.str() };
        preview->setSheets(best.sheets);
        preview->resetView();
    }
    catch (const std::exception&)
    {
        if (calculationRevision == revision)
            summary->setText("Unable to calculate this layout. Try fewer pieces.");
    }
}
