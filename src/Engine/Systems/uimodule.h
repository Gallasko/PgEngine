#pragma once

#include "uisystem.h"

#include "Interpreter/pginterpreter.h"

namespace pg
{
    class SetX : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem* ecsRef, const CompRef<UiComponent>& comp)
        {
            setArity(1, 1);

            this->ecsRef = ecsRef;
            this->comp = comp;
        }

        virtual ValuablePtr call(ValuableQueue& args) const override
        {
            auto x = args.front()->getElement();
            args.pop();

            comp->setX(x.get<float>());

            return nullptr;
        }

        EntitySystem* ecsRef = nullptr;
        CompRef<UiComponent> comp;
    };

    class SetY : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem* ecsRef, const CompRef<UiComponent>& comp)
        {
            setArity(1, 1);

            this->ecsRef = ecsRef;
            this->comp = comp;
        }

        virtual ValuablePtr call(ValuableQueue& args) const override
        {
            auto y = args.front()->getElement();
            args.pop();

            comp->setY(y.get<float>());

            return nullptr;
        }

        EntitySystem* ecsRef = nullptr;
        CompRef<UiComponent> comp;
    };

    class SetW : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem* ecsRef, const CompRef<UiComponent>& comp)
        {
            setArity(1, 1);

            this->ecsRef = ecsRef;
            this->comp = comp;
        }

        virtual ValuablePtr call(ValuableQueue& args) const override
        {
            auto w = args.front()->getElement();
            args.pop();

            comp->setWidth(w.get<float>());

            return nullptr;
        }

        EntitySystem* ecsRef = nullptr;
        CompRef<UiComponent> comp;
    };

    class SetH : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem* ecsRef, const CompRef<UiComponent>& comp)
        {
            setArity(1, 1);

            this->ecsRef = ecsRef;
            this->comp = comp;
        }

        virtual ValuablePtr call(ValuableQueue& args) const override
        {
            auto h = args.front()->getElement();
            args.pop();

            comp->setHeight(h.get<float>());

            return nullptr;
        }

        EntitySystem* ecsRef = nullptr;
        CompRef<UiComponent> comp;
    };

    std::vector<SysListElement> uiElementFunctionsList(const Function *caller, EntitySystem *ecsRef, const CompRef<UiComponent>& comp);

    struct UiModule : public SysModule
    {
        UiModule(EntitySystem *ecsRef)
        {
        }
    };

}