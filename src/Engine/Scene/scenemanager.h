#pragma once

#include "sceneloader.h"

namespace pg
{
    struct CreateSceneElement { CreateSceneElement(EntityRef entity) : entity(entity) {} EntityRef entity; };

    struct SceneElement
    {
        SceneElement(Entity *entity) : entity(entity) {}

        Entity *entity;
    };

    // Todo objectiv with system implementation
    // struct SceneElementSystem : public System<Policy<ExecutionPolicy::Manual>, Own<SceneElement>, Need<MasterRenderer>, Talk<SceneSystem>>
    struct SceneElementSystem : public System<Own<SceneElement>, StoragePolicy>
    {
        SceneElementSystem() {}
        
        template <typename Type, typename... Args>
        void addComponent(SceneElement *element, Args... args)
        {
            // Todo disable every thing on the entity except Rendering to not update the scene element during editing !
            ecsRef->attach<Type>(element->entity, std::forward<Args>(args)...);
        }
    };
}