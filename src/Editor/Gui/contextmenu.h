#pragma once

#include <functional>

#include "UI/uisystem.h"

namespace pg {
    enum class UiComponentType
    {
        BUTTON,
        TEXTURE,
        TEXT,
        TEXTINPUT,
        LIST,
        PREFAB // TOdo to implement !
    };

    namespace ecs
    {
        class EntitySystem;
    }

    // Class forwarding
    class FontLoader;
    class MasterRenderer;

namespace editor
{
    struct ContextMenu : public UiComponent
    {
        ContextMenu(EntitySystem &ecs, FontLoader *fontLoader, const std::string& textureName, const std::function<void(const UiComponentType&)>& callback);
        ~ContextMenu();

        virtual void render(MasterRenderer* masterRenderer);

        void show() override;
        void hide() override;

        UiComponent *backgroundTextureC;

        UiComponent *addButtonButtonC;
        UiComponent *addTextureButtonC;
        UiComponent *addTextButtonC;
        UiComponent *addTextInputButtonC;
        UiComponent *addListButtonC;
        UiComponent *addPrefabButtonC;

        std::function<void(const UiComponentType&)> callback;
    };
}
}