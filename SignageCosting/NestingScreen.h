#pragma once

#include "Screen.h"
#include "NavigationPanel.h"
#include "TextBox.h"
#include "Label.h"
#include "NestingCore.h"
#include "NestingService.h"
#include "Material.h"
#include <future>
#include <array>
#include <filesystem>

class NestingPreviewPanel;
class NestingWorkspacePanel;
class NestingSizeSwatch;
class Button;
class ComboBox;
class NestingPageButton;
class NestingOutlineEditor;

class NestingScreen : public Screen
{
public:
    explicit NestingScreen(std::shared_ptr<NavigationPanel> navigation);
    void onActivated() override;
    void tick(float deltaTime) override;
    void update(const SDL_Event& event) override;
    void render(Renderer& renderer) override;

private:
    void calculate(bool capacity = false);
    void refreshStockOptions();
    void selectStockVariant(int index, const std::string& preferredSize = {});
    void selectStockSize(int index, bool preserveRollLength = false);
    void invalidate();
    void addSize();
    void arrangeWorkspace();
    void paintWorkspace(Renderer& renderer);
    struct SizeRow
    {
        NestingGeometry::PartId partId = NestingGeometry::newPartId();
        std::shared_ptr<Panel> panel;
        std::shared_ptr<TextBox> width, height, quantity;
        std::shared_ptr<NestingSizeSwatch> swatch;
        NestingGeometry::Polygon outline = NestingGeometry::rectangle(1,1);
        std::string shapeName = "Rectangle";
        std::shared_ptr<NestingPageButton> shapeButton;
    };
    void editOutline(const std::shared_ptr<SizeRow>& row);
    bool customMode() const;
    int sizeRowHeight() const;
    void refreshGeometryMode();
    void chooseDxf();
    void importDxfFile(const std::filesystem::path& path);
    std::shared_ptr<NavigationPanel> navigation;
    std::shared_ptr<TextBox> sheetWidth, sheetHeight, cuttingGap, sheetMargin;
    std::shared_ptr<ComboBox> stockVariant, stockSize, capacityMode;
    struct StockVariant
    {
        std::vector<SheetFormat> formats;
        bool roll = false;
    };
    std::vector<StockVariant> stockOptions;
    std::shared_ptr<Panel> sizes;
    std::vector<std::shared_ptr<SizeRow>> rows;
    std::shared_ptr<Label> summary, details;
    std::shared_ptr<NestingPreviewPanel> preview;
    std::shared_ptr<NestingWorkspacePanel> workspace;
    std::shared_ptr<Button> addButton, importButton, calculateButton, capacityButton;
    bool capacityCalculation = false;
    std::shared_ptr<Panel> rotationControls;
    std::shared_ptr<ComboBox> geometrySelector, rotationSelector;
    std::shared_ptr<NestingOutlineEditor> outlineEditor;
    std::array<std::string, 4> metricValues{ "—", "—", "—", "—" };
    SDL_Rect setupBounds{}, metricsBounds{}, tableBounds{};
    bool rebuildRows = false;
    unsigned revision = 0, calculationRevision = 0;
    double calculationArea = 0.0, calculationSheetArea = 0.0;
    size_t calculationSizeCount = 0;
    std::future<NestingService::Result> calculation;
};
