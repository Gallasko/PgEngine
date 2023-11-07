#pragma once

#include "2D/simple2dobject.h"

#include "Interpreter/pginterpreter.h"

#include "uimodule.h"

namespace pg
{
    class CreateRectangle : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem* ecsRef)
        {
            setArity(2, 2);

            this->ecsRef = ecsRef;
        }

        virtual ValuablePtr call(ValuableQueue& args) const override
        {
            auto x = args.front()->getElement();
            args.pop();

            auto y = args.front()->getElement();
            args.pop();

            std::cout << "[Interpreter]: Creating rec at: (" << x.toString() << ", " << y.toString() << ")" << std::endl;

            auto rec = makeSimple2DShape(ecsRef, Shape2D::Square, 50, 50, {255.0f, 0.0f, 0.0f});
            auto recUi = rec.get<UiComponent>();

            if(x.isNumber() and y.isNumber())
            {
                recUi->setX(x.get<float>());
                recUi->setY(y.get<float>());
            }
            else
            {
                LOG_ERROR("CreateRectangle", "Cannot create a new rectangle values passed are not numbers");
            }

            std::cout << "[Interpreter]: Creating rec at: (" << x.toString() << ", " << y.toString() << ")" << std::endl;

            auto list = makeList(this, {});

            auto res = addToList(this, list, uiElementFunctionsList(this, ecsRef, recUi));

            return res;
        }

        EntitySystem* ecsRef = nullptr;
    };

    struct Shape2DModule : public SysModule
    {
        Shape2DModule(EntitySystem *ecsRef)
        {
            addSystemFunction<CreateRectangle>("renderSquare", ecsRef);
        }
    };
}