#pragma once

#include <unordered_map>
#include <Memory/memorypool.h>

#include "uniqueid.h"

namespace pg
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
        Entity() : id(0), ecsRef(nullptr) {}

        // Default copy constructor
        // Entity(Entity& mE)              = default;
        // Entity& operator=(Entity& mE)   = default;

        inline bool has(const _unique_id& otherId) const { return componentList.find(otherId) != componentList.end(); }

        // todo
        // template <typename Component>
        // inline bool has() const { return ecsRef && has(ecsRef->has<Component>(this)); }
        template <typename Component>
        inline bool has() const { return false; }

        template <typename Component>
        inline Component* get() { return has<Component>() ? static_cast<Component* >(componentList[Component::componentId]) : nullptr; }
    
        inline const EntitySystem* world() const { return ecsRef; }

        _unique_id id;

        // Todo make this mutable because it is only used for memoisation purposes
        std::unordered_map<_unique_id, Entity*> componentList;

        //Todo overload operator delete to call ecsRef->deleteEntity(this);

    protected:
        Entity(_unique_id id, EntitySystem* ecs) : id(id), ecsRef(ecs) {}
        ~Entity() { }

        // Todo use this destructor but set ecsRef to nullptr when calling it from deleteEntity of the ecs to not destroy the entity multiple time
        // ~Entity() { if(ecsRef) ecsRef->deleteEntity(this); }

        const EntitySystem *ecsRef = nullptr;
    };
}