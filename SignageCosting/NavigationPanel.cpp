#include "NavigationPanel.h"
#include "NavigationItem.h"
#include "VerticalLayout.h"

NavigationPanel::NavigationPanel()
{
    setLayout(std::make_unique<VerticalLayout>());

    setSidebarStyle(true);
}

std::shared_ptr<NavigationItem> NavigationPanel::addItem(
    const std::string& text,
    std::function<void()> callback,
    const std::string& iconPath)
{
    auto item = std::make_shared<NavigationItem>();

    item->setText(text);

    item->setIcon(iconPath);

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

    addLayoutElement(
        item,
        SizePolicy::Fixed,
        60);

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
    auto heading = std::make_shared<NavigationItem>();

    heading->setText(text);

    // Headings are purely visual.
    // They are not selectable and have no callback.
    heading->setSelected(false);

    addLayoutElement(
        heading,
        SizePolicy::Fixed,
        40);
}