#include "componenttab.h"

#include <string>

#include "ECS/entitysystem.h"
#include "UI/button.h"
#include "2D/texture.h"
#include "Renderer/renderer.h"
#include "Scene/scenemanager.h"

namespace pg
{
    namespace
    {
        static constexpr char const * DOM = "Component Tab";
    }

namespace editor
{
    ComponentTab::ComponentTab()
    {
        LOG_THIS_MEMBER(DOM);

    }

    ComponentTab::~ComponentTab()
    {
        LOG_THIS_MEMBER(DOM);
    }

    void ComponentTab::init()
    {
        LOG_THIS_MEMBER(DOM);

        parent = ecsRef->createEntity();

        parentUi = ecsRef->attach<UiComponent>(parent);

        parentUi->setZ(0);

        auto backTexture = makeUiTexture(ecsRef, 1, 1, "TabTexture");

        auto background = backTexture.entity;

        backgroundC = backTexture.get<UiComponent>();

        backgroundC->fill(parentUi);

        parentUi->update();
    }

    void ComponentTab::hide()
    {
        LOG_THIS_MEMBER(DOM);

        parentUi->hide();

        backgroundC->hide();
    }
}
}
