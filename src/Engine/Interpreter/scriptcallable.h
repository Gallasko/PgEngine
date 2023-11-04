#pragma once

#include "ECS/callable.h"

#include "interpreter.h"

namespace pg
{
    struct CallableIntepretedFunction : public AbstractCallable
    {
        CallableIntepretedFunction(std::shared_ptr<Function> fun)
        {
            fun->getVisitor()->setEcsSysFlag();

            visitorRef = fun->getVisitor()->getVisitorRef();

            function = std::make_shared<Function>(visitorRef, fun);
        }

        virtual ~CallableIntepretedFunction() {}

        inline virtual void call(EntitySystem* const) override
        {
            ValuableQueue emptyQueue;

            function->getValue(emptyQueue);
        }

        inline void call(std::queue<std::shared_ptr<Valuable>>& args)
        {
            function->getValue(args);
        }

        inline Function* getRef() const { return function.get(); }

        std::shared_ptr<Function> function;

        std::shared_ptr<VisitorReference> visitorRef;
    };

    std::shared_ptr<CallableIntepretedFunction> makeCallable(std::shared_ptr<Function> fun);
}