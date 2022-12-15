#pragma once

#include <vector>
#include <algorithm>
#include <Memory/memorypool.h>

#include "uniqueid.h"

namespace pg
{
    class EntitySystem;

    class Entity
    {
    friend class EntitySystem;
    friend class AllocatorPool<Entity>;
    public:
        // Todo remove this as it is only for testing purposes
        // 0 means that it can't be a valid entity !
        Entity() : id(0), ecsRef(nullptr) {}

        // Default copy and move constructor
        // Entity(Entity& mE)              = default;
        // Entity& operator=(Entity& mE)   = default;
        // Entity(Entity&& mE)             = default;
        // Entity& operator=(Entity&& mE)  = default;
        
        inline bool has(const _unique_id& otherId) const noexcept
        {
            return std::find_if(
                componentList.begin(),
                componentList.end(),
                [&otherId](Entity *ent){ return ent->id == otherId;}) !=
                componentList.end();
        }

        // todo
        // template <typename Component>
        // inline bool has() const { return ecsRef && has(ecsRef->has<Component>(this)); }
        template <typename Comp>
        inline bool has() const noexcept;

        template <typename Comp>
        inline Comp* get() noexcept;
    
        inline const EntitySystem* world() const noexcept { return ecsRef; }

        _unique_id id;

        // Todo make this mutable because it is only used for memoisation purposes
        // std::unordered_map<_unique_id, Entity*> componentList;
        std::vector<Entity*> componentList;

        //Todo overload operator delete to call ecsRef->deleteEntity(this);

    protected:
        Entity(_unique_id id, EntitySystem* ecs) noexcept : id(id), ecsRef(ecs) {}
        ~Entity() noexcept { }

        // Todo use this destructor but set ecsRef to nullptr when calling it from deleteEntity of the ecs to not destroy the entity multiple time
        // ~Entity() { if(ecsRef) ecsRef->deleteEntity(this); }

        const EntitySystem *ecsRef = nullptr;
    };
}