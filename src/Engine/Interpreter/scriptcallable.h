#pragma once

#include "ECS/callable.h"

#include "interpreter.h"

namespace pg
{
    struct CallableIntepretedFunction : public AbstractCallable
    {
        CallableIntepretedFunction(std::shared_ptr<Function> fun) : function(fun) {}
        virtual ~CallableIntepretedFunction() {}

        inline virtual void call(EntitySystem* const) override
        {
            ValuableQueue emptyQueue;

            auto m = function->getVisitor()->getMutex();

            std::lock_guard lock(*m);
            function->getValue(emptyQueue);
        }

        std::shared_ptr<Function> function;
    };

    std::shared_ptr<CallableIntepretedFunction> makeCallable(std::shared_ptr<Function> fun);
}