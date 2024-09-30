#pragma once

#include "coresystems.h"

#include "Interpreter/pginterpreter.h"

namespace pg
{
    class ConnectToTick : public Function
    {
        using Function::Function;
    private:
        struct TickListener
        {
            TickListener(std::shared_ptr<Function> func) : function(makeCallable(func)) {}

            void onEvent(const TickEvent& event)
            {
                if (not function)
                    return;

                ValuableQueue queue;
                auto arg = makeList(function->getRef(), {{"tick", static_cast<int>(event.tick)}});

                queue.push(arg);

                try
                {
                    function->call(queue);
                }
                catch(const std::exception& e)
                {
                    LOG_ERROR("Core Module", e.what());
                }
            }

            std::shared_ptr<CallableIntepretedFunction> function;
        };

    public:
        virtual ~ConnectToTick() { for(auto listener : tickListeners) delete listener; }

        void setUp(ComponentRegistry *registryRef)
        {
            LOG_THIS_MEMBER("Core Module");

            setArity(1, 1);

            this->registryRef = registryRef;
        }

        virtual ValuablePtr call(ValuableQueue& args) override
        {
            LOG_THIS_MEMBER("Core Module");

            auto arg = args.front();
            args.pop();

            if (arg->getType() == "Function")
            {
                auto fun = std::static_pointer_cast<Function>(arg);

                auto listener = new TickListener(fun);

                registryRef->addEventListener<TickEvent>(listener);

                tickListeners.push_back(listener);
            }
            else
            {
                LOG_ERROR("Core Module", "Need to pass a function to be able to listen to an event");
            }

            return nullptr; 
        }

    private:
        ComponentRegistry *registryRef;
        
        mutable std::vector<TickListener*> tickListeners;
    };

    class EntityNameToId : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem *ecsRef)
        {
            LOG_THIS_MEMBER("Ecs Module");

            setArity(1, 1);

            this->ecsRef = ecsRef;
        }

        virtual ValuablePtr call(ValuableQueue& args) override
        {
            LOG_THIS_MEMBER("Ecs Module");

            auto name = args.front()->getElement();
            args.pop();

            if (not name.isLitteral())
            {
                LOG_ERROR("EntityNameToId", "Cannot get entity id, values passed are not of the correct type." <<
                          "\nExpected (litteral) instead got:  " << name.getTypeString() << "!");

                return makeVar(0);
            }

            auto nameSys = ecsRef->getSystem<EntityNameSystem>();

            auto id = nameSys->getEntityId(name.toString());

            return makeVar(id); 
        }

        EntitySystem *ecsRef;
    };

    class GetAllNamedEntities : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem *ecsRef)
        {
            LOG_THIS_MEMBER("Core Module");

            setArity(0, 0);

            this->ecsRef = ecsRef;
        }

        virtual ValuablePtr call(ValuableQueue&) override
        {
            LOG_THIS_MEMBER("Core Module");

            auto entityList = makeList(this, {});

            auto nameSys = ecsRef->getSystem<EntityNameSystem>();

            const auto& list = nameSys->view<EntityName>();

            for (auto ent : list)
            {
                addToList(entityList, this->token, {ent->name, ent->entityId});
            }

            return entityList; 
        }

        EntitySystem *ecsRef;
    };

    struct CoreModule : public SysModule
    {
        CoreModule(EntitySystem *ecsRef)
        {
            LOG_THIS_MEMBER("Core Module");

            addSystemFunction<ConnectToTick>("connectToTick", &(ecsRef->registry));
            addSystemFunction<EntityNameToId>("entityNameToId", ecsRef);
            addSystemFunction<GetAllNamedEntities>("getAllNamedEntities", ecsRef);
        }
    };
}