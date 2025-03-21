#pragma once

#include "skilltree.h"
#include "customitem.h"
#include "custompassive.h"

namespace pg
{
    struct SlashSpell : public Spell
    {
        SlashSpell()
        {
            name = "Slash";

            baseDmg = 5;

            physicalMultipler = 1.0f;
        }
    };

    struct FireSlash : public Spell
    {
        FireSlash()
        {
            name = "Fire Slash";

            baseDmg = 5;

            physicalMultipler = 1.0f;

            PassiveCall call;

            call.passiveName = "Burn";

            call.info.type = PassiveType::CharacterEffect;

            call.info.trigger = TriggerType::TurnStart;

            call.args["burnDmg"] = 5;

            applyToTarget.push_back(call);
        }
    };

    struct WarriorTree : public SkillTree
    {
        WarriorTree()
        {
            name = "Warrior";

            currentLevel = 0;

            maxLevel = 10;

            for (size_t i = 0; i < maxLevel; i++)
                requiredMatForNextLevel.neededMat[i] = { XpStone { i + 1 } };
        
            for (size_t i = 0; i <= maxLevel; i++)
            {
                if (i % 5 == 0)
                {
                    levelGains[i].stats.health += 50;
                    levelGains[i].stats.physicalAttack += 1;
                    levelGains[i].stats.physicalDefense += 1;
                }
                else
                {
                    levelGains[i].stats.health += 10;
                }
            }

            levelGains[0].learntSpells = { SlashSpell{} };

            levelGains[1].learntSpells = { FireSlash{} };
        }
    };

    struct FireBall : public Spell
    {
        FireBall()
        {
            name = "FireBall";
        }
    };

    struct MageTree : public SkillTree
    {
        MageTree()
        {
            name = "Mage";

            currentLevel = 0;

            maxLevel = 10;

            for (size_t i = 0; i < maxLevel; i++)
                requiredMatForNextLevel.neededMat[i] = { XpStone { i + 1 } };
        
            for (size_t i = 0; i <= maxLevel; i++)
            {
                if (i % 5 == 0)
                {
                    levelGains[i].stats.magicalAttack += 5;
                }
                else
                {
                    levelGains[i].stats.magicalAttack += 1;
                }
            }

            levelGains[0].learntSpells = { FireBall{} };
        }
    };

    struct AdventurerTree : public SkillTree
    {
        AdventurerTree()
        {
            name = "Adventurer";

            currentLevel = 0;

            maxLevel = 10;

            for (size_t i = 0; i < maxLevel; i++)
            {
                if (i == 0)
                {
                    requiredMatForNextLevel.neededMat[i] = { XpStone { i + 1 }, SlimeBall{} };
                }
                else
                {
                    requiredMatForNextLevel.neededMat[i] = { XpStone { i + 1 } };
                }
                
            }

            for (size_t i = 0; i <= maxLevel; i++)
            {
                if (i % 5 == 0)
                {
                    levelGains[i].stats.health += 10;
                }
                else
                {
                    levelGains[i].stats.health += 5;
                }
            }

            levelGains[0].learntSpells = { };
        }
    };
}