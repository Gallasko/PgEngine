#pragma once

#include "sceneloader.h"

#include "Input/inputcomponent.h"

namespace pg
{
    struct SceneElementClicked { SceneElementClicked(EntityRef entity) : entity(entity) {} EntityRef entity; };

    struct SceneElement { };

    // Todo objectiv with system implementation
    // struct SceneElementSystem : public System<Policy<ExecutionPolicy::Manual>, Own<SceneElement>, Need<MasterRenderer>, Talk<SceneSystem>>
    struct SceneElementSystem : public System<Listener<SceneElementClicked>, Own<SceneElement>, StoragePolicy, InitSys>
    {
        SceneElementSystem() {}

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
        
        template <typename Type, typename... Args>
        void addComponent(SceneElement *element, Args... args)
        {
            // Todo disable every thing on the entity except Rendering to not update the scene element during editing !
            // ecsRef->attach<Type>(element->entity, std::forward<Args>(args)...);
        }
    };
}