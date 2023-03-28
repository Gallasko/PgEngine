#include "componentregistry.h"

namespace pg
{
    ComponentRegistry::ComponentRegistry(EntitySystem *ecs) : ecsRef(ecs)
    {
        LOG_THIS_MEMBER("ComponentRegistry");
    } 

    ComponentRegistry::~ComponentRegistry()
    {
        LOG_THIS_MEMBER("Component Registry");

        for(auto group : groupStorageMap)
            delete group.second;

        // delete masterRenderer;
    }
}