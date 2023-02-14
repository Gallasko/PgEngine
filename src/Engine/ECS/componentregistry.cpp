#include "componentregistry.h"

#include "Renderer/renderer.h"

namespace pg
{
    ComponentRegistry::ComponentRegistry(EntitySystem *ecs) : masterRenderer(new MasterRenderer()), ecsRef(ecs)
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