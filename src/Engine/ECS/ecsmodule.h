#pragma once

#include "entitysystem.h"

#include "Interpreter/pginterpreter.h"
#include "Interpreter/interpretersystem.h"

namespace pg
{
    class GetAllSystemFunction : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem *ecsRef)
        {
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

    class GetAllEntityFunction : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem *ecsRef)
        {
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

    class RegisterNewSystem : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem *ecsRef)
        {
            setArity(1, 1);

            this->ecsRef = ecsRef;
        }

        virtual ValuablePtr call(ValuableQueue& args) const override
        {
            auto arg = args.front();
            args.pop();

            if(arg->getType() == "ClassInstance")
            {
                auto sys = std::static_pointer_cast<ClassInstance>(arg);

                ecsRef->createSystem<InterpreterSystem>(env, sys);
            }

            return nullptr; 
        }

        EntitySystem *ecsRef;
    };

    class NewUniqueId : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem *ecsRef)
        {
            setArity(0, 0);

            this->ecsRef = ecsRef;
        }

        virtual ValuablePtr call(ValuableQueue&) const override
        {
            return makeVar(ecsRef->generateId());
        }

        EntitySystem *ecsRef;
    };

    struct EcsModule : public SysModule
    {
        EcsModule(EntitySystem *ecsRef)
        {
            addSystemFunction<GetAllSystemFunction>("getAllSystems", ecsRef);
            addSystemFunction<GetAllEntityFunction>("getAllEntities", ecsRef);
            addSystemFunction<RegisterNewSystem>("registerSystem", ecsRef);
            addSystemFunction<NewUniqueId>("generateNewId", ecsRef);
        }
    };

}