#pragma once

#include "entitysystem.h"

#include "Interpreter/pginterpreter.h"

namespace pg
{
    class GetAllEntityFunction : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem *ecsRef)
        {
            // Todo make the type of the texture as an optional arg
            // setArity(2, 3);
    
            setArity(0, 0);

            this->ecsRef = ecsRef;
        }

        virtual ValuablePtr call(ValuableQueue&) const override
        {
            auto entityList = makeList(this, {});

            for(auto entity : ecsRef->view())
            {
                auto compList = makeList(this, {});
                
                size_t j = 0;
                for(const auto& compId : entity->componentList)
                {
                    addToList(compList, this->token, {std::to_string(j), compId.getId()});
                    j++;
                }

                addToList(entityList, this->token, {std::to_string(entity->id), compList});
            }

            return entityList; 
        }

        EntitySystem *ecsRef;
    };

    struct EcsModule : public SysModule
    {
        EcsModule(EntitySystem *ecsRef)
        {            
            addSystemFunction<GetAllEntityFunction>("getAllEntities", ecsRef);
        }
    };

}