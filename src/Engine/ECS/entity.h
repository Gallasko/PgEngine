#pragma once

#include <unordered_map>
#include <Memory/memorypool.h>

#include "uniqueid.h"

namespace pg
{
    namespace ecs
    {
        class AbstractComponent;
        class EntitySystem;

        class Entity
        {
        friend class EntitySystem;
        friend class AllocatorPool<Entity>;
        public:
            // Todo remove this as it is only for testing purposes
            // 0 means that it can't be a valid entity !
            Entity() : id(0) {}

            // Default copy constructor
            // Entity(Entity& mE)              = default;
            // Entity& operator=(Entity& mE)   = default;

            inline bool has(const _unique_id& otherId) const { return componentList.find(otherId) != componentList.end(); }

            template <typename Component>
            inline bool has() const { return has(Component::componentId); }

            template <typename Component>
            inline Component* get() { return has<Component>() ? static_cast<Component* >(componentList[Component::componentId]) : nullptr; }
        
            inline const EntitySystem* world() const { return ecsRef; }

            _unique_id id;

            // Todo make this mutable because it is only used for memoisation purposes
            std::unordered_map<_unique_id, Entity*> componentList;

            //Todo overload operator delete to call ecsRef->deleteEntity(this);

        protected:
            Entity(_unique_id id) : id(id) {}
            ~Entity() {}

            const EntitySystem *ecsRef = nullptr;
        };
    }
}