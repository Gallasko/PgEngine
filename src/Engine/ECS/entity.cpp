#include "entity.h"

#include "entitysystem.h"

#include "logger.h"

namespace pg
{
    namespace
    {
        constexpr static const char * const DOM = "Entity";
    }

    void EntityRef::operator=(const EntityRef& rhs)
    {
        LOG_THIS_MEMBER(DOM);

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

                // Todo see if we propagate back the finding of the entity to the base ref !
                // rhs.entity = entity
                // rhs.initialized = true
                // Note that it needs to make the rhs not const or we need to make the member entity mutable !
            }
            else
            {
                LOG_ERROR(DOM, "Copy of a reference to an invalid entity");

                initialized = rhs.initialized;
                entity      = rhs.entity;
                ecsRef      = rhs.ecsRef;
            }
        }
    }

    Entity* EntityRef::operator->() const
    {
        if(initialized)
            return ecsRef->getEntity(id);
        else
            return entity;
    }

    EntityRef::operator Entity*() const
    {
        if(initialized)
            return ecsRef->getEntity(id);
        else
            return entity;
    }
}