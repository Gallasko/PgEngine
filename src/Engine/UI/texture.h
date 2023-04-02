#pragma once

#include "uisystem.h"
#include "constant.h"

#include "logger.h"

namespace pg
{
    class Renderable;

    struct TextureChangeEvent
    {
        _unique_id id;
        std::string oldTextureName;
        std::string newTextureName;
    };

    struct TextureComponent : public Ctor
    {
        TextureComponent(const std::string& textureName) : textureName(textureName) { }
        TextureComponent(const TextureComponent &rhs) : textureName(rhs.textureName) { }
        virtual ~TextureComponent() {}

        virtual void onCreation(Entity* entity) { this->entity = entity; }

        inline void setTexture(const std::string& textureName)
        {
            if(entity)
                entity->world()->sendEvent(TextureChangeEvent{entity->id, this->textureName, textureName});
            
            this->textureName = textureName;
        }

        std::string textureName;

        Entity *entity = nullptr;
    };

    struct TextureComponentSystem : public System<Own<TextureComponent>, Ref<UiComponent>, NamedSystem, InitSys, StoragePolicy>
    {
        TextureComponentSystem() { }

        virtual std::string getSystemName() const override { return "Texture System"; }

        virtual void init() override;
    };

    /** Helper that create an entity with an Ui component and a Texture component */
    CompList<UiComponent, TextureComponent> makeUiTexture(EntitySystem *ecs, float width, float height, const std::string& name);

}