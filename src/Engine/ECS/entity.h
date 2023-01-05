#pragma once

#include <vector>
#include <algorithm>
#include <Memory/memorypool.h>

#include "uniqueid.h"

namespace pg
{
    // Todo find the correct place for this
    template <class T>struct tag{using type=T;};

    class EntitySystem;

    class Entity
    {
    friend class EntitySystem;
    friend class AllocatorPool<Entity>;
    
    struct EntityHeld
    {
        union EntityHeldId
        {
            explicit EntityHeldId(Entity *ent) : entity(ent) {}
            explicit EntityHeldId(_unique_id id) : id(id) {}

            Entity *entity;
            _unique_id id;
        };

        enum class EntityHeldType
        {
            entity,
            id
        };

        explicit EntityHeld(Entity* entity) : entityHeldId(entity), entityHeldType(EntityHeldType::entity) { }
        explicit EntityHeld(_unique_id id) : entityHeldId(id), entityHeldType(EntityHeldType::id) {}

        constexpr bool operator==(_unique_id id) const
        {
            if(entityHeldType == EntityHeldType::entity)
                return entityHeldId.entity->id == id;
            else
                return entityHeldId.id == id;
        }

        void operator=(Entity* entity)
        {
            entityHeldId.entity = entity;
            entityHeldType = EntityHeldType::entity;
        }

        void operator=(_unique_id id)
        {
            entityHeldId.id = id;
            entityHeldType = EntityHeldType::id;
        }

        void operator=(const EntityHeld& rhs)
        {
            entityHeldId = rhs.entityHeldId;
            entityHeldType = rhs.entityHeldType;
        }

        _unique_id getId() const
        {
            
        }

        EntityHeldId entityHeldId;
        EntityHeldType entityHeldType;
    };

    public:
        // Todo remove this as it is only for testing purposes
        // 0 means that it can't be a valid entity !
        Entity() : id(0), ecsRef(nullptr) {}

        // Default copy and move constructor
        // Entity(Entity& mE)              = default;
        // Entity& operator=(Entity& mE)   = default;
        // Entity(Entity&& mE)             = default;
        // Entity& operator=(Entity&& mE)  = default;
        
        inline  bool has(const _unique_id& otherId) const noexcept
        {
            return std::find(componentList.begin(), componentList.end(), otherId) != componentList.end();
        }

        // todo
        // template <typename Component>
        // inline bool has() const { return ecsRef && has(ecsRef->has<Component>(this)); }
        template <typename Comp>
        inline bool has() const noexcept;

        template <typename Comp>
        inline Comp* get() noexcept;
    
        inline EntitySystem* world() const noexcept { return ecsRef; }

        _unique_id id;

        // Todo make this mutable because it is only used for memoisation purposes
        // std::unordered_map<_unique_id, Entity*> componentList;
        std::vector<EntityHeld> componentList;

        //Todo overload operator delete to call ecsRef->deleteEntity(this);

    protected:
        Entity(_unique_id id, EntitySystem* ecs) noexcept : id(id), ecsRef(ecs) {}
        ~Entity() noexcept { }

        // Todo use this destructor but set ecsRef to nullptr when calling it from deleteEntity of the ecs to not destroy the entity multiple time
        // ~Entity() { if(ecsRef) ecsRef->deleteEntity(this); }

        EntitySystem *const ecsRef = nullptr;
    };
}