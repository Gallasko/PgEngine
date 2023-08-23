#include "componentregistry.h"

#include "Interpreter/interpretersystem.h"

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

    void ComponentRegistry::addEventListener(_unique_id eventId, InterpreterSystem *listener)
    {
        LOG_THIS_MEMBER("Component Registry");
        
        eventStorageMap[eventId].emplace_back([eventId, listener](const AbstractEvent& event)
        {
            struct DelegateEvent : public AbstractEvent, public std::shared_ptr<ClassInstance> { virtual ~DelegateEvent() {} };

            listener->onEvent(eventId, static_cast<const std::shared_ptr<ClassInstance>&>(static_cast<const DelegateEvent&>(event)));
        });
    }
}