#pragma once

#include <chrono>

#include "logger.h"

#include "ECS/entitysystem.h"

#include "Renderer/renderer.h"
#include "UI/sentencesystem.h"

using namespace pg;

// Todo add all the logger thing to all those systems and doc too
struct TickEvent
{
    TickEvent(size_t duration) : tick(duration) {}

    size_t tick;
};

struct TickingSystem : public System<NamedSystem>
{
    TickingSystem(int16_t duration = 40) : tickDuration(duration), reminder(0)
    { 
        LOG_THIS_MEMBER("Ticking System");
        
        // Todo replace QDateTime with std::chrono
        // firstTickTime = std::chrono::high_resolution_clock::now();
        firstTickTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        secondTickTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    ~TickingSystem() { LOG_THIS_MEMBER("Ticking System"); stop(); }

    virtual std::string getSystemName() const override { return "Ticking System"; }

    inline void stop()
    {
        LOG_THIS_MEMBER("Ticking System");

        LOG_INFO("Ticking System", "Ticking system stopping ...");

        paused = false;
    }

    inline void pause()
    {
        LOG_THIS_MEMBER("Ticking System");

        paused = true;
    }

    inline void resume()
    {
        LOG_THIS_MEMBER("Ticking System");

        firstTickTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        secondTickTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        paused = false;
    }

    // Todo test this step by step to see if the reminder is correctly calculated
    virtual void execute()
    {
        LOG_THIS_MEMBER("Ticking System");

        bool triggered = false;

        secondTickTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        // To prevent for an overflow
        if(secondTickTime < firstTickTime)
        {
            LOG_MILE("Ticking System", "Overflow detected, reset counters");

            firstTickTime = secondTickTime;
            reminder = 0;
            return;
        }

        auto delta = secondTickTime - firstTickTime - reminder;
        
        while(not paused and (delta >= tickDuration))
        {
            triggered = true;

            delta -= tickDuration;

            ecsRef->sendEvent(TickEvent{tickDuration});
        }

        if(triggered)
        {
            firstTickTime = secondTickTime;

            reminder = delta;
            
            // This should never happend
            if(reminder < 0)
            {
                LOG_ERROR("Ticking System", "Anormal reminder of less than 0 (" << reminder << ")");
                reminder = 0;
            }
        }
    }

    int16_t tickDuration;

    int16_t firstTickTime, secondTickTime, reminder; 
    bool paused = false;
};

struct FpsSystem : public System<Listener<TickEvent>, NamedSystem, InitSys, StoragePolicy>
{
    virtual std::string getSystemName() const override { return "Fps System"; }

    void init() override
    {
        auto sentence = makeSentence(ecsRef, 0, 0, {"0"});

        fpsTextId = sentence.entity.id;
    }

    void onEvent(const TickEvent& event) override
    {
        LOG_THIS_MEMBER("FactorySystem");

        accumulatedTick += event.tick;

        if (accumulatedTick >= 1000)
        {
            accumulatedTick %= 1000;

            auto rendererSys = ecsRef->getSystem<MasterRenderer>();

            if (not rendererSys)
                return;

            auto currentNbOfFrames = rendererSys->getNbRenderedFrames();

            // In case of overflow of size_t
            if(currentNbOfFrames < lastNbOfFrames)
            {
                lastNbOfFrames = currentNbOfFrames;
                return;
            }

            auto res = currentNbOfFrames - lastNbOfFrames;

            auto fpsStr = Strfy() << res;

            lastNbOfFrames = currentNbOfFrames;

            // Print FPS
            ecsRef->sendEvent(OnTextChanged{fpsTextId, fpsStr.getData()});
        }
    }

    _unique_id fpsTextId;
    size_t accumulatedTick = 0;
    size_t lastNbOfFrames = 0;
};

struct OnClickGainGold { };

struct OnGoldGain
{
    OnGoldGain(int64_t gold) : gold(gold) {}

    int64_t gold;
};

struct GoldSystem : public System<Listener<OnClickGainGold>, Listener<OnGoldGain>, NamedSystem, InitSys, StoragePolicy>
{
    virtual std::string getSystemName() const override { return "Gold System"; }

    void init() override
    {
        auto sentence = makeSentence(ecsRef, 150, 20, {"0"});

        goldTextId = sentence.entity.id;
    }

    void onEvent(const OnClickGainGold&) override
    {
        this->gold += clickPower;

        auto goldStr = Strfy() << this->gold.load();

        ecsRef->sendEvent(OnTextChanged{goldTextId, goldStr.getData()});
    }

    void onEvent(const OnGoldGain& event) override
    {
        this->gold += event.gold;

        auto goldStr = Strfy() << this->gold.load();

        ecsRef->sendEvent(OnTextChanged{goldTextId, goldStr.getData()});
    }

    int64_t clickPower = 1;

    std::atomic<int64_t> gold {0};
    _unique_id goldTextId;
};

struct BuyFactory { };

struct FactorySystem : public System<Listener<BuyFactory>, Listener<TickEvent>, NamedSystem, StoragePolicy>
{
    virtual std::string getSystemName() const override { return "Factory System"; }

    void onEvent(const BuyFactory&) override
    {
        LOG_THIS_MEMBER("FactorySystem");

        LOG_INFO("FactorySystem", "Bought a new factory");
        nbFactory += 1;
    }

    void onEvent(const TickEvent& event) override
    {
        LOG_THIS_MEMBER("FactorySystem");

        accumulatedTick += event.tick;

        while (accumulatedTick >= factoryProdDuration)
        {
            LOG_INFO("FactorySystem", "Factory produced gold");

            accumulatedTick -= factoryProdDuration;

            ecsRef->sendEvent(OnGoldGain{static_cast<int64_t>(nbFactory * factoryProdValue)});
        }
    }

    size_t accumulatedTick = 0;

    size_t nbFactory = 0;
    size_t factoryProdDuration = 1000;
    size_t factoryProdValue = 1;
};
