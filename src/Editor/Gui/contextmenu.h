#pragma once

#include "UI/uisystem.h"

namespace pg {
    enum class UiComponentType
    {
        BUTTON,
        TEXTURE,
        TEXT,
        LIST,
        PREFAB
    };

    // Class forwarding
    class EntitySystem;
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
        UiComponent *addListButtonC;
    };
}
}