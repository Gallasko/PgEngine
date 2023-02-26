#pragma once

#include "uisystem.h"
#include "constant.h"

#include "logger.h"

namespace pg
{
    class MasterRenderer;
    class Renderable;

    struct TextureComponent
    {
        TextureComponent(const std::string& textureName) : textureName(textureName) { }
        TextureComponent(const TextureComponent &rhs) : textureName(rhs.textureName) { }
        virtual ~TextureComponent() {}

        inline void setTexture(const std::string& textureName) { this->textureName = textureName; initialised = false; }

        std::string textureName;

        bool initialised = false;
    };

    struct TextureComponentSystem : public System<Own<TextureComponent>, Ref<UiComponent>, StoragePolicy, InitSys>
    {
        TextureComponentSystem() { }

        virtual void init() override
        {
            auto group = registerGroup<UiComponent, TextureComponent>();

            group->addOnGroup([](Entity* entity) {
                LOG_INFO("Texture Component System", "Add entity " << entity->id << " to ui - tex group !");

                entity->world()->attach<Renderable>(entity);
            });
        }

    };

}