#pragma once

#include "ECS/system.h"

#include "UI/listview.h"

#include "Scene/scenemanager.h"

#include "character.h"

#include "location.h"

#include "Systems/coresystems.h"


namespace pg
{
    struct SpellCasted
    {
        SpellCasted() {}
        SpellCasted(size_t caster, std::vector<Character*> ids, Spell *spell) : caster(caster), ids(ids), spell(spell) {}

        size_t caster = 0;

        std::vector<Character*> ids = {nullptr};
        
        Spell *spell = nullptr;
    };

    struct SelectedSpell
    {
        SelectedSpell(Spell *spell) : spell(spell) {}

        Spell *spell;  
    };

    struct FightSystemUpdate {};

    struct StartFight {};

    struct PlayerNextTurn
    {
        Character* chara;
    };

    struct EnemyNextTurn
    {
        Character* chara;
    };

    struct DeadPlayerEvent { Character chara; };

    enum class FightAnimationEffects : uint8_t
    {
        Nothing = 0, // Used only to advance the state machine (when no spell is casted or the enemy couldn't target anyone !)
        Hit,
        Heal,
        StatusAilement,
        Counter,
        Dodge,
    };

    struct PlayFightAnimation
    {
        PlayFightAnimation(size_t id, const FightAnimationEffects& effect) : id(id), effect(effect) {}

        size_t id;

        FightAnimationEffects effect;
    };

    struct PlayFightAnimationDone {};

    struct StartFightAtLocation
    {
        StartFightAtLocation(const Location& location, const std::vector<Character>& characters) : location(location), characters(characters) {}
        StartFightAtLocation(const StartFightAtLocation& other) : location(other.location), characters(other.characters) {}

        Location location;

        std::vector<Character> characters;
    };

    enum class FightState : uint8_t
    {
        Unknown = 0,
        Start,
        StartTurn,
        CastSpell,
        Animation,
        NextTurn,
        Wait,
        End,
    };

    struct FightSystem : public System<Listener<StartFightAtLocation>, Listener<StartFight>, Listener<SpellCasted>, Listener<PlayFightAnimationDone>, Listener<EnemyNextTurn>, Listener<DeadPlayerEvent>, InitSys>
    {
        virtual void onEvent(const StartFight& event) override;
        virtual void onEvent(const EnemyNextTurn& event) override;
        virtual void onEvent(const PlayFightAnimationDone& event) override;
        virtual void onEvent(const SpellCasted& event) override;
        virtual void onEvent(const StartFightAtLocation& event) override;
        virtual void onEvent(const DeadPlayerEvent& event) override;

        virtual void init() override;

        void clear();

        virtual void execute() override;

        void resolveSpell(size_t casterId, size_t receiverId, Spell* spell);

        void processNextTurn();

        void skipTurn(size_t id = 0, const FightAnimationEffects& effect = FightAnimationEffects::Nothing);

        void addCharacter(Character character);

        void calculateNextPlayingCharacter();

        void startTurn();

        void tickDownPassives(Character* character);

        void checkEndFight();

        void processWin();
        void processLose();

        Character* findNextPlayingCharacter();

        Character* currentPlayingCharacter = nullptr;

        std::vector<Character> characters;

        bool needToProcessEnemyNextTurn = false;

        bool spellToBeResolved = false;
        SpellCasted spellToResolve;

        bool inEncounter = false;
        Location currentLocation;
        Encounter currentEncounter;

        FightState currentState = FightState::Unknown;
        CompRef<Timer> timer;

        size_t fightSpeed = 300.0f;
    };

    struct FightMessageEvent
    {
        FightMessageEvent(const std::string& message) : message(message) {}

        std::string message;
    };

    struct FightScene : public Scene
    {
        virtual void init() override;

        virtual void startUp() override;

        virtual void execute() override;

        void castSpell();

        void writeInLog(const std::string& message);

        void updateHealthBars();

        FightSystem *fightSys;

        std::unordered_map<std::string, std::vector<EntityRef>> uiElements;

        CompRef<ListView> logView;

        bool needHealthBarUpdate = false;

        std::vector<Character*> selectedTarget;

        std::vector<PlayFightAnimation> animationToDo;
    };
}