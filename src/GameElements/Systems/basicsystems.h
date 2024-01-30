#pragma once

#include "logger.h"

#include "Systems/coresystems.h"

using namespace pg;
struct FpsSystem : public System<Listener<TickEvent>, NamedSystem, InitSys, StoragePolicy>
{
    virtual std::string getSystemName() const override { return "Fps System"; }

    virtual void init() override
    {
        LOG_INFO("FPS System initialized", "FPS System initializing");

        auto sentence = makeSentence(ecsRef, 500, 0, {"0"});

        fpsTextId = sentence.entity.id;

        LOG_INFO("FPS System initialized", "Sentence id: " << fpsTextId);
    }

    virtual void onEvent(const TickEvent& event) override
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
    OnGoldGain(size_t gold) : gold(gold) {}

    size_t gold;
};

struct GoldSystem : public System<Listener<OnClickGainGold>, Listener<OnGoldGain>, NamedSystem, InitSys, StoragePolicy>
{
    virtual std::string getSystemName() const override { return "Gold System"; }

    void init() override
    {
        auto sentence = makeSentence(ecsRef, 150, 20, {"0"});

        goldTextId = sentence.entity.id;
    }

    virtual void onEvent(const OnClickGainGold&) override
    {
        this->gold += clickPower;

        auto goldStr = Strfy() << this->gold.load();

        ecsRef->sendEvent(OnTextChanged{goldTextId, goldStr.getData()});
    }

    virtual void onEvent(const OnGoldGain& event) override
    {
        this->gold += event.gold;

        auto goldStr = Strfy() << this->gold.load();

        ecsRef->sendEvent(OnTextChanged{goldTextId, goldStr.getData()});
    }

    size_t clickPower = 1;

    std::atomic<size_t> gold {0};
    _unique_id goldTextId;
};

struct BuyFactory { };

struct FactorySystem : public System<Listener<BuyFactory>, Listener<TickEvent>, NamedSystem, StoragePolicy>
{
    virtual std::string getSystemName() const override { return "Factory System"; }

    virtual void onEvent(const BuyFactory&) override
    {
        LOG_THIS_MEMBER("FactorySystem");

        LOG_INFO("FactorySystem", "Bought a new factory");
        nbFactory += 1;
    }

    virtual void onEvent(const TickEvent& event) override
    {
        LOG_THIS_MEMBER("FactorySystem");

        accumulatedTick += event.tick;

        while (accumulatedTick >= factoryProdDuration)
        {
            LOG_INFO("FactorySystem", "Factory produced gold");

            accumulatedTick -= factoryProdDuration;

            ecsRef->sendEvent(OnGoldGain{static_cast<size_t>(nbFactory * factoryProdValue)});
        }
    }

    size_t accumulatedTick = 0;

    size_t nbFactory = 0;
    size_t factoryProdDuration = 1000;
    size_t factoryProdValue = 1;
};
