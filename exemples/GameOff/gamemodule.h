#pragma once

#include "Interpreter/pginterpreter.h"

#include "character.h"
#include "item.h"
#include "locationscene.h"

namespace pg
{
    class CreateCharacter : public Function
    {
        using Function::Function;
    public:
        void setUp()
        {
            setArity(1, 1);
        }

        virtual ValuablePtr call(ValuableQueue& args) override
        {
            auto v = args.front()->getElement();
            args.pop();

            if (not v.isLitteral())
            {
                LOG_ERROR("Character module", "Character name should be a literal");

                return nullptr;
            }

            Character chara;

            chara.type = CharacterType::Enemy;

            chara.name = v.toString();

            auto list = serializeToInterpreter(this, chara);

            return list;
        }
    };

    class CreateItem : public Function
    {
        using Function::Function;
    public:
        void setUp()
        {
            setArity(1, 1);
        }

        virtual ValuablePtr call(ValuableQueue& args) override
        {
            auto v = args.front()->getElement();
            args.pop();

            if (not v.isLitteral())
            {
                LOG_ERROR("Character module", "Character name should be a literal");

                return nullptr;
            }

            Item item;

            item.name = v.toString();

            auto list = serializeToInterpreter(this, item);

            return list;
        }
    };

    class ReadCharaList : public Function
    {
        using Function::Function;
    public:
        void setUp()
        {
            setArity(1, 1);
        }

        virtual ValuablePtr call(ValuableQueue& args) override
        {
            auto arg = args.front();
            args.pop();

            auto charaList = deserializeTo<Encounter>(arg);

            for (const auto& chara : charaList.characters)
            {
                LOG_INFO("Module", "Chara in list: " << chara.name);
            }

            return nullptr;
        }
    };

    class CreateLocation : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem *ecsRef)
        {
            this->ecsRef = ecsRef;

            setArity(2, 2);
        }

        virtual ValuablePtr call(ValuableQueue& args) override
        {
            auto locationName = args.front()->getElement();
            args.pop();

            auto arg = args.front();
            args.pop();

            if (arg->getType() == "ClassInstance")
            {
                // Todo create an helper funciton for this cast
                auto charaObject = std::static_pointer_cast<ClassInstance>(arg);

                // auto sysMethods = sysInstance->getMethods();
                auto sysFields = charaObject->getFields();

                Location location;
                location.name = locationName.toString();

                for (auto& field : sysFields)
                {
                    auto type = field.value->getType();
                    LOG_INFO("Game Module", type);

                    auto encounter = deserializeTo<Encounter>(field.value);

                    location.possibleEnounters.push_back(encounter);

                    LOG_INFO("Game Module", "nbChara: " <<encounter.characters.size());
                }

                ecsRef->getSystem<LocationSystem>()->locations.push_back(location);
            }

            return nullptr;
        }

        EntitySystem *ecsRef;
    };

    class CreatePassive : public Function
    {
        using Function::Function;
    public:
        void setUp()
        {
            setArity(1, 1);
        }

        virtual ValuablePtr call(ValuableQueue& args) override
        {
            auto v = args.front()->getElement();
            args.pop();

            if (not v.isLitteral())
            {
                LOG_ERROR("Character module", "Character name should be a literal");

                return nullptr;
            }

            auto list = makeList(this, {
                {"name", v.toString()},
                {"nbTurn", -1},
                {"hidden", false},
                {"nbNeededTrigger", 0},
                {"trigger", "TurnStart"},
                {"type", "SpellEffect"}
                });

            // ApplyOn and RemoveFrom are functions

            return list;
        }
    };

    class CreateSpell : public Function
    {
        using Function::Function;
    public:
        void setUp()
        {
            setArity(1, 1);
        }

        virtual ValuablePtr call(ValuableQueue& args) override
        {
            auto v = args.front()->getElement();
            args.pop();

            if (not v.isLitteral())
            {
                LOG_ERROR("Character module", "Character name should be a literal");

                return nullptr;
            }

            Spell spell;

            auto list = serializeToInterpreter(this, spell);

            return list;
        }
    };

    struct GameModule : public SysModule
    {
        GameModule(EntitySystem *ecsRef)
        {
            addSystemFunction<CreateCharacter>("newChara");
            addSystemFunction<CreateLocation>("createLocation", ecsRef);
            addSystemFunction<CreateItem>("newItem");
            addSystemFunction<CreateSpell>("newSpell");
            addSystemFunction<CreatePassive>("createPassive");

            addSystemFunction<ReadCharaList>("readCharaList");
        }
    };


}