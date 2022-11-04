#include "componentregistry.h"

#include "Renderer/renderer.h"
#include "Input/inputcomponent.h"

namespace pg
{
    namespace ecs
    {
        ComponentRegistry::ComponentRegistry() : inputSystem(new InputSystem), masterRenderer(new MasterRenderer())
        {
        } 

        ComponentRegistry::~ComponentRegistry()
        {
            LOG_THIS_MEMBER("Component Registry");

            for(auto group : groupStorageMap)
                delete group.second;

            delete inputSystem;

            delete masterRenderer;
        }

    }
}