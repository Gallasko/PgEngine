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
            TickListener(std::shared_ptr<Function> func) : function(func) {}

            void onEvent(const TickEvent& event)
            {
                if(not function)
                    return;

                ValuableQueue queue;
                auto arg = makeList(function.get(), {{"tick", static_cast<int>(event.tick)}});

                queue.push(arg);

                try
                {
                    auto m = function->getVisitor()->getMutex();
                    m->lock();
                    function->getValue(queue);
                    m->unlock();
                }
                catch(const std::exception& e)
                {
                    LOG_ERROR("Core Module", e.what());
                }
            }

            std::shared_ptr<Function> function;
        };

    public:
        virtual ~ConnectToTick() { for(auto listener : tickListeners) delete listener; }

        void setUp(ComponentRegistry *registryRef)
        {
            LOG_THIS_MEMBER("Core Module");

            setArity(1, 1);

            this->registryRef = registryRef;
        }

        virtual ValuablePtr call(ValuableQueue& args) const override
        {
            LOG_THIS_MEMBER("Core Module");

            auto arg = args.front();
            args.pop();

            if(arg->getType() == "Function")
            {
                auto fun = std::static_pointer_cast<Function>(arg);

                auto listener = new TickListener(fun);

                registryRef->addEventListener<TickEvent>(listener);

                tickListeners.push_back(listener);

                visitor->setEcsSysFlag();
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

    struct CoreModule : public SysModule
    {
        CoreModule(EntitySystem *ecsRef)
        {
            LOG_THIS_MEMBER("Core Module");

            addSystemFunction<ConnectToTick>("connectToTick", &(ecsRef->registry));
        }
    };
}