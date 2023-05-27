#pragma once

#include "ECS/entitysystem.h"

#include "ECS/callable.h"
namespace pg 
{
    enum class UiComponentType
    {
        BUTTON,
        TEXTURE,
        TEXT,
        TEXTINPUT,
        LIST,
        PREFAB // Todo to implement !
    };

    class UiComponent;
    class Input;

namespace editor
{
    struct ShowContextMenu 
    {
        ShowContextMenu(Input* inputHandler, const CompRef<UiComponent>& sceneUi) : inputHandler(inputHandler), sceneUi(sceneUi) {}

        Input* inputHandler; CompRef<UiComponent> sceneUi;
    };

    struct HideContextMenu {};

    struct CreateElement { CreateElement(const UiComponentType& type) : type(type) {} UiComponentType type; };

    // Todo make the context menu a generic Ui element !
    struct ContextMenu : System<Listener<ShowContextMenu>, Listener<HideContextMenu>, Listener<CreateElement>, InitSys, StoragePolicy>
    {
        ContextMenu();
        ~ContextMenu();

        virtual void init() override;

        void setContextList(const std::string& text, CallablePtr callable);

        template<typename... Args>
        void setContextList(const std::string& text, CallablePtr callable, const std::string& nText, CallablePtr nCallable, Args... args)
        {
            addItemInContextMenu(text, callable);

            setContextList(nText, nCallable, args...);
        }

        void addItemInContextMenu(const std::string& text, CallablePtr callable);

        inline void clear() { components.clear(); }

        void hide();

        virtual void onEvent(const ShowContextMenu& event) override;
        virtual void onEvent(const HideContextMenu&) override;
        virtual void onEvent(const CreateElement& event) override;

        EntityRef parent;
        CompRef<UiComponent> parentUi;

        CompRef<UiComponent> backgroundC;

        std::vector<CompRef<UiComponent>> components;

        float currentX = 0.0f, currentY = 0.0f; 
    };
}
}