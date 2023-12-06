#pragma once

#include "sceneloader.h"

#include "Input/inputcomponent.h"

#include "serialization.h"

namespace pg
{
    struct SceneElementClicked { SceneElementClicked(EntityRef entity) : entity(entity) {} EntityRef entity; };

    struct SaveScene { };

    struct LoadScene { std::string filename; };

    struct NameScene { std::string filename; };

    struct SceneElement : public Ctor
    {
        virtual void onCreation(EntityRef ent) override { entity = ent; }
        
        EntityRef entity;
    };

    // Todo objectiv with system implementation
    // struct SceneElementSystem : public System<Policy<ExecutionPolicy::Manual>, Own<SceneElement>, Need<MasterRenderer>, Talk<SceneSystem>>
    struct SceneElementSystem : public System<
        Listener<SceneElementClicked>, Listener<SaveScene>, Listener<LoadScene>, Listener<NameScene>,
        Own<SceneElement>, StoragePolicy, InitSys>
    {
        SceneElementSystem()
        {
            currentScene = "newScene.sz";
            serializer.setFile("newScene.sz");
        }

        virtual void init() override
        {
            auto group = registerGroup<UiComponent, SceneElement>();

            group->addOnGroup([](EntityRef entity) {
                LOG_INFO("Scene Element System", "Add entity " << entity->id << " to ui - scene group !");

                auto entityUiC = entity->get<UiComponent>();

                auto overlappingEntity = entity->world()->createEntity();

                auto uiComp = entity->world()->attach<UiComponent>(overlappingEntity);

                uiComp->fill(entityUiC);

                LOG_INFO("Scene Element System", "Add entity ui (" << entity->id << "): x = " << static_cast<UiSize>(uiComp->pos.x) << ", y = " << static_cast<UiSize>(uiComp->pos.y) << ", w = " << static_cast<UiSize>(uiComp->width) << ", h = " << static_cast<UiSize>(uiComp->height));

                uiComp->setZ(entityUiC->pos.z + 1);

                entity->world()->attach<MouseLeftClickComponent>(overlappingEntity, makeCallable<SceneElementClicked>(entity));
            });
        }

        virtual void onEvent(const SceneElementClicked& event) override
        {
            LOG_INFO("Scene Element", "Entity clicked: " << event.entity.id);
        }

        virtual void onEvent(const SaveScene& event) override
        {
            LOG_INFO("Scene Element", "Save current scene named: " << currentScene);

            for (const auto& comp : view<SceneElement>())
            {
                if (not comp->entity.empty())
                    serializer.serializeObject(std::to_string(comp->entity.id), *comp->entity.entity);
            }
        }

        virtual void onEvent(const LoadScene& event) override
        {
            LOG_INFO("Scene Element", "Load scene: " << event.filename);

            for (const auto& comp : view<SceneElement>())
            {
                ecsRef->removeEntity(comp->entity);
            }

            std::unordered_map<_unique_id, _unique_id> idCorrelationMap;

            serializer.setFile(currentScene);

            for (const auto& elem : serializer.getSerializedMap())
            {
            }
        }

        virtual void onEvent(const NameScene& event) override
        {
            LOG_INFO("Scene Element", "Name the current scene: " << event.filename);

            currentScene = event.filename;

            serializer.setFile(currentScene);
        }
        
        template <typename Type, typename... Args>
        void addComponent(SceneElement *element, Args... args)
        {
            // Todo disable every thing on the entity except Rendering to not update the scene element during editing !
            // ecsRef->attach<Type>(element->entity, std::forward<Args>(args)...);
        }

        std::string currentScene;

        Serializer serializer;
    };
}