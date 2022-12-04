#include "componentregistry.h"

#include "Renderer/renderer.h"

namespace pg
{
    namespace ecs
    {
        ComponentRegistry::ComponentRegistry() : masterRenderer(new MasterRenderer())
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
}