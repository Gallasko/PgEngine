#pragma once

#include "sentencesystem.h"

#include "Interpreter/pginterpreter.h"

namespace pg
{
    /*class NewUniqueIdFromString : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem *ecsRef)
        {
            LOG_THIS_MEMBER("Ecs Module");

            setArity(1, 1);

            this->ecsRef = ecsRef;
        }

        virtual ValuablePtr call(ValuableQueue& args) const override
        {
            LOG_THIS_MEMBER("Ecs Module");

            auto arg = args.front();
            args.pop();

            auto key = arg->getValue()->getElement().toString();

            const auto& it = uniqueIds.find(key);

            if(it != uniqueIds.end())
                return makeVar(it->second);
            else
            {
                auto id = ecsRef->generateId();
                uniqueIds.emplace(key, id);
                return makeVar(id);
            }
        }

        EntitySystem *ecsRef;
        mutable std::unordered_map<std::string, _unique_id> uniqueIds;
    };


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

            return makeList(this, {
                {"x", static_cast<UiSize>(recUi->pos.x)},
                {"y", static_cast<UiSize>(recUi->pos.y)},
                {"w", recUi->width},
                {"h", recUi->height},
                {"setX", makeFun<SetX>(this, "setX", ecsRef, recUi)},
                {"setY", makeFun<SetY>(this, "setY", ecsRef, recUi)},
                {"setWidth", makeFun<SetW>(this, "setWidth", ecsRef, recUi)},
                {"setHeight", makeFun<SetH>(this, "setHeight", ecsRef, recUi)}});
        }

        EntitySystem* ecsRef = nullptr;
    };

    struct SentenceModule : public SysModule
    {
        SentenceModule(EntitySystem *ecsRef)
        {
            LOG_THIS_MEMBER("Core Module");

            addSystemFunction<ConnectToTick>("connectToTick", &(ecsRef->registry));
        }
    };*/

}