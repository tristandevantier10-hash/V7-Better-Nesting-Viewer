#include "ApplicationShell.h"

#include "Panel.h"
#include "NavigationPanel.h"

#include "VerticalLayout.h"
#include "HorizontalLayout.h"

ApplicationShell::ApplicationShell()
{
    setLayout(std::make_unique<VerticalLayout>());

    headerPanel = std::make_shared<Panel>();
    headerPanel->setLayout(std::make_unique<HorizontalLayout>());
    headerPanel->setHeaderStyle(true);

    addLayoutElement(
        headerPanel,
        SizePolicy::Fixed,
        100);

    bodyPanel = std::make_shared<Panel>();
    bodyPanel->setLayout(std::make_unique<HorizontalLayout>());

    addLayoutElement(
        bodyPanel,
        SizePolicy::Fill);

    navigationPanel = std::make_shared<NavigationPanel>();
    navigationPanel->setLayout(std::make_unique<VerticalLayout>());
    navigationPanel->setSidebarStyle(true);

    bodyPanel->addLayoutElement(
        navigationPanel,
        SizePolicy::Fixed,
        200);

    contentPanel = std::make_shared<Panel>();
    contentPanel->setLayout(std::make_unique<VerticalLayout>());

    bodyPanel->addLayoutElement(
        contentPanel,
        SizePolicy::Fill);
}

void ApplicationShell::setContent(std::shared_ptr<UIElement> content)
{
    contentPanel->clearChildren();

    if (content)
    {
        contentPanel->addLayoutElement(
            content,
            SizePolicy::Fill);

    }
}