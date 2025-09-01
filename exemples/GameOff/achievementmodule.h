#pragma once

#include "Interpreter/pginterpreter.h"
#include "nexusscene.h"

namespace pg
{
    class CreateFactChecker : public Function
    {
        using Function::Function;
    public:
        void setUp() { setArity(3, 3); }

        FactChecker createFactChecker(const std::string& name, const ElementType& value, const std::string& equalityStr)
        {
            auto equalityIt = stringToEquality.find(equalityStr);
            FactCheckEquality equality = (equalityIt != stringToEquality.end()) ? equalityIt->second : FactCheckEquality::None;
            return FactChecker(name, value, equality);
        }

        virtual ValuablePtr call(ValuableQueue& args) override
        {
            // Todo check type of elements gotten here
            // Assume arguments: name (string), value (ElementType), equality (string)
            std::string name = args.front()->getElement().toString();
            args.pop();
            ElementType value = args.front()->getElement();
            args.pop();
            std::string equalityStr = args.front()->getElement().toString();
            args.pop();

            FactChecker fc = createFactChecker(name, value, equalityStr);
            return serializeToInterpreter(this, fc);
        }
    };

    class CreateAchievementRewardEvent : public Function
    {
        using Function::Function;
    public:
        void setUp() { setArity(3, -1); }

        virtual ValuablePtr call(ValuableQueue& args) override
        {
            // Todo check type of elements gotten here
            // Assume arguments: eventName (string), key (string), message (ElementType)
            std::string eventName = args.front()->getElement().toString();
            args.pop();
            std::string key = args.front()->getElement().toString();
            args.pop();

            const auto& message = args.front()->getElement();
            StandardEvent ev(eventName, key, message);
            args.pop();

            while (not args.empty())
            {
                std::string key = args.front()->getElement().toString();
                args.pop();

                if (args.empty())
                    throw RuntimeException(token, "Invalid number of arguments for function call: '" + token.text + "' expected value after a key .");

                ev.values[key] = args.front()->getElement();
                args.pop();
            }

            AchievementReward reward(ev);

            return serializeToInterpreter(this, reward);
        }
    };

    class CreateAchievementAddFact: public Function
    {
        using Function::Function;
    public:
        void setUp() { setArity(2, 2); }

        virtual ValuablePtr call(ValuableQueue& args) override
        {
            // Todo check type of elements gotten here
            // Assume arguments: eventName (string), message (ElementType)
            std::string eventName = args.front()->getElement().toString();
            args.pop();

            const auto& message = args.front()->getElement();

            AchievementReward reward(AddFact{eventName, message});
            args.pop();

            return serializeToInterpreter(this, reward);
        }
    };

    class CreateAchievementRemoveFact: public Function
    {
        using Function::Function;
    public:
        void setUp() { setArity(1, 1); }

        virtual ValuablePtr call(ValuableQueue& args) override
        {
            // Todo check type of elements gotten here
            // Assume arguments: eventName (string)
            std::string eventName = args.front()->getElement().toString();
            args.pop();

            AchievementReward reward(RemoveFact{eventName});

            return serializeToInterpreter(this, reward);
        }
    };

    class CreateAchievementIncreaseFact: public Function
    {
        using Function::Function;
    public:
        void setUp() { setArity(2, 2); }

        virtual ValuablePtr call(ValuableQueue& args) override
        {
            // Todo check type of elements gotten here
            // Assume arguments: eventName (string), value (ElementType)
            std::string eventName = args.front()->getElement().toString();
            args.pop();

            const auto& value = args.front()->getElement();

            AchievementReward reward(IncreaseFact{eventName, value});
            args.pop();

            return serializeToInterpreter(this, reward);
        }
    };

    struct AchievementModule : public SysModule
    {
        AchievementModule()
        {
            addSystemFunction<CreateFactChecker>("FactChecker");
            addSystemFunction<CreateAchievementRewardEvent>("AchievementRewardEvent");

            // Todo maybe rename it AchievementAddFact
            addSystemFunction<CreateAchievementAddFact>("AddFact");
            addSystemFunction<CreateAchievementRemoveFact>("RemoveFact");
            addSystemFunction<CreateAchievementIncreaseFact>("IncreaseFact");
            // add additional functions as needed
        }
    };
}

