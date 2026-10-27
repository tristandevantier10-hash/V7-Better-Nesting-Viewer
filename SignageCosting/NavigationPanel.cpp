#include "NavigationPanel.h"
#include "NavigationItem.h"
#include "VerticalLayout.h"
#include "Metrics.h"
#include "Renderer.h"
#include "Theme.h"
#include <algorithm>
#include <cctype>

namespace
{
    class NavigationBrand : public UIElement
    {
    public:
        void render(Renderer& renderer) override
        {
            if (!visible)
                return;

            const SDL_Rect imageRect =
            {
                bounds.x + 12,
                bounds.y + 8,
                bounds.w - 24,
                bounds.h - 16
            };
            renderer.drawImage("Assets/Images/EstiMate-White.png", imageRect, true);
        }
    };

    class DashboardNavigationBrand : public UIElement
    {
    public:
        void render(Renderer& renderer) override
        {
            if (!visible)
                return;

            const SDL_Rect brandBand
            {
                bounds.x - Metrics::SidebarPadding,
                bounds.y - Metrics::SidebarPadding,
                bounds.w + Metrics::SidebarPadding * 2,
                bounds.h + Metrics::SidebarPadding + Metrics::SpaceS
            };
            renderer.fillRect(brandBand, DefaultTheme.sidebarBackground);

            const SDL_Rect logoRect
            {
                brandBand.x + 20,
                brandBand.y + 18,
                brandBand.w - 40,
                brandBand.h - 40
            };
            renderer.drawImage("Assets/Images/EstiMate-White.png", logoRect, true);
            renderer.fillRect(
                { brandBand.x, brandBand.y + brandBand.h - 2, brandBand.w, 2 },
                DefaultTheme.accent);
        }
    };
}

NavigationPanel::NavigationPanel()
{
    setLayout(std::make_unique<VerticalLayout>());

    setSidebarStyle(true);
}

void NavigationPanel::addBrand()
{
    if (hasBrand)
        return;

    hasBrand = true;
    addLayoutElement(std::make_shared<NavigationBrand>(), SizePolicy::Fixed, 76);
}

void NavigationPanel::addDashboardBrand()
{
    if (hasBrand)
        return;

    hasBrand = true;
    addLayoutElement(
        std::make_shared<DashboardNavigationBrand>(),
        SizePolicy::Fixed,
        116);
}

std::shared_ptr<NavigationItem> NavigationPanel::addItem(
    const std::string& text,
    std::function<void()> callback,
    const std::string& iconPath,
    bool enabled)
{
    auto item = std::make_shared<NavigationItem>();

    item->setText(text);

    item->setIcon(iconPath);
    item->setEnabled(enabled);

    item->setOnClick(
        [this, item, callback]()
        {
            selectItem(item);

            // Run the user's callback
            if (callback)
                callback();
        });

    const std::string lowerText = [&text]()
    {
        std::string value = text;
        std::transform(value.begin(), value.end(), value.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return value;
    }();
    std::string icon = iconPath;
    if (icon.empty())
    {
        if (lowerText.find("new job") != std::string::npos || lowerText == "job input") icon = "Assets/Icons/new_job.svg";
        else if (lowerText.find("material") != std::string::npos) icon = "Assets/Icons/materials.svg";
        else if (lowerText.find("quote") != std::string::npos || lowerText == "invoice") icon = "Assets/Icons/quotes.svg";
        else if (lowerText.find("customer") != std::string::npos) icon = "Assets/Icons/customers.svg";
        else if (lowerText.find("labour") != std::string::npos) icon = "Assets/Icons/labour.svg";
        else if (lowerText.find("setting") != std::string::npos) icon = "Assets/Icons/settings.svg";
        else if (lowerText.find("exit") != std::string::npos) icon = "Assets/Icons/exit.svg";
        else if (lowerText.find("back") != std::string::npos || lowerText.find("cancel") != std::string::npos) icon = "Assets/Icons/back.svg";
        else if (lowerText.find("save") != std::string::npos) icon = "Assets/Icons/save.svg";
    }
    item->setIcon(icon);

    addLayoutElement(item, SizePolicy::Fixed, Metrics::NavigationItemHeight);

    items.push_back(item);

    return item;
}

void NavigationPanel::selectItem(const std::shared_ptr<NavigationItem>& item)
{
    for (auto& navigationItem : items)
        navigationItem->setSelected(navigationItem == item);
}

void NavigationPanel::selectItemByText(const std::string& text)
{
    selectItem(findItemByText(text));
}

std::shared_ptr<NavigationItem> NavigationPanel::findItemByText(
    const std::string& text) const
{
    const auto found = std::find_if(items.begin(), items.end(),
        [&text](const auto& item) { return item->getText() == text; });
    return found == items.end() ? nullptr : *found;
}

void NavigationPanel::addHeading(
    const std::string& text)
{
    auto heading = std::make_shared<NavigationItem>();

    heading->setText(text);
    heading->setHeading(true);

    // Headings are purely visual.
    // They are not selectable and have no callback.
    heading->setSelected(false);

    addLayoutElement(
        heading,
        SizePolicy::Fixed,
        40);
}
