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
                bounds.y + 10,
                bounds.w - 24,
                bounds.h - 20
            };
            renderer.drawImage("Assets/Images/EstiMate.png", imageRect, true);
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
    auto brand = std::make_shared<NavigationBrand>();
    addLayoutElement(brand, SizePolicy::Fixed, 82);
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
            // Deselect every item
            for (auto& i : items)
            {
                i->setSelected(false);
            }

            // Select this one
            item->setSelected(true);

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

    if (items.size() == 1)
    {
        item->setSelected(true);
    }

    return item;
}

void NavigationPanel::addHeading(
    const std::string& text)
{
    addBrand();

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
