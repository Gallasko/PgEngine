#pragma once

#include "coresystems.h"

#include "Interpreter/pginterpreter.h"

namespace pg
{
    class OnTimerTimeout : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem* ecsRef, CompRef<Timer> comp)
        {
            setArity(1, 1);

            this->ecsRef = ecsRef;
            this->comp = comp;
        }

        virtual ValuablePtr call(ValuableQueue& args) override
        {
            auto arg = args.front();
            args.pop();

            if(arg->getType() == "Function")
            {
                auto fun = std::static_pointer_cast<Function>(arg);

                comp->callback = makeCallable(fun);

                visitor->setEcsSysFlag();
            }
            else
            {
                LOG_ERROR("Input Module", "Need to pass a function to be able to listen to an event");
            }

            return nullptr;
        }

        EntitySystem* ecsRef = nullptr;
        CompRef<Timer> comp;
    };

    class TimerInterval : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem* ecsRef, CompRef<Timer> comp)
        {
            setArity(1, 1);

            this->ecsRef = ecsRef;
            this->comp = comp;
        }

        virtual ValuablePtr call(ValuableQueue& args) override
        {
            auto arg = args.front()->getElement();
            args.pop();

            if(not arg.isNumber())
            {
                LOG_ERROR("Timer Module", "Need to pass a number to set an interval on a timer");
                return nullptr;
            }

            comp->interval = arg.get<size_t>();

            return nullptr;
        }

        EntitySystem* ecsRef = nullptr;
        CompRef<Timer> comp;
    };

    class TimerStart : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem* ecsRef, CompRef<Timer> comp)
        {
            setArity(0, 1);

            this->ecsRef = ecsRef;
            this->comp = comp;
        }

        virtual ValuablePtr call(ValuableQueue& args) override
        {
            if(args.size() != 0)
            {
                auto arg = args.front()->getElement();
                args.pop();

                if(not arg.isNumber())
                {
                    LOG_ERROR("Timer Module", "Need to pass a number to set an interval on a timer");
                    return nullptr;
                }

                comp->interval = arg.get<size_t>();
            }

            if(comp->interval == 0)
            {
                LOG_ERROR("Timer Module", "Need to set an interval to use a timer");
            }
            else
            {
                comp->running = true;
            }

            return nullptr;
        }

        EntitySystem* ecsRef = nullptr;
        CompRef<Timer> comp;
    };

    class TimerStop : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem* ecsRef, CompRef<Timer> comp)
        {
            setArity(0, 0);

            this->ecsRef = ecsRef;
            this->comp = comp;
        }

        virtual ValuablePtr call(ValuableQueue&) override
        {
            comp->running = false;

            return nullptr;
        }

        EntitySystem* ecsRef = nullptr;
        CompRef<Timer> comp;
    };

    class CreateTimer : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem* ecsRef)
        {
            setArity(0, 0);

            this->ecsRef = ecsRef;
        }

        virtual ValuablePtr call(ValuableQueue&) override
        {
            auto entity = ecsRef->createEntity();

            auto timer = ecsRef->attach<Timer>(entity);

            auto list = makeList(this, {
                {"id", entity.id},
                {"onTimeout", makeFun<OnTimerTimeout>(this, "onTimeout", ecsRef, timer)},
                {"setInterval", makeFun<TimerInterval>(this, "setInterval", ecsRef, timer)},
                {"start", makeFun<TimerStart>(this, "start", ecsRef, timer)},
                {"stop", makeFun<TimerStop>(this, "stop", ecsRef, timer)}});

            return list;
        }

        EntitySystem* ecsRef = nullptr;
    };

    struct TimeModule : public SysModule
    {
        TimeModule(EntitySystem *ecsRef)
        {
            addSystemFunction<CreateTimer>("newTimer", ecsRef);
        }
    };

}