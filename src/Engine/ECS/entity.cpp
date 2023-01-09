#include "entity.h"

#include "entitysystem.h"

namespace pg
{
    void EntityRef::operator=(const EntityRef& rhs)
    {
        if(rhs.initialized)
        {
            initialized = rhs.initialized;
            entity      = rhs.entity;
            id          = rhs.id;
            ecsRef      = rhs.ecsRef;
        }
        else
        {
            id = rhs.id;

            if(id != 0)
            {
                entity = rhs.ecsRef->getEntity(id);
                ecsRef = rhs.ecsRef;
                initialized = true;
            }
            else
            {
                entity = nullptr;
                ecsRef = nullptr;
                initialized = false;
            }

            // Todo see if we propagate back the finding of the entity to the base ref !
            // rhs.entity = entity
            // Note that it needs to make the rhs not const or we need to make the member entity mutable !
        }
    }
}