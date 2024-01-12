#pragma once

#include <chrono>
#include "ECS/entitysystem.h"

#include "Renderer/renderer.h"
#include "UI/sentencesystem.h"

#include "Interpreter/scriptcallable.h"

namespace pg
{
    // Todo add all the logger thing to all those systems and doc too
    struct TickEvent
    {
        TickEvent(int16_t duration) : tick(duration) {}

        int16_t tick;
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
        virtual void execute() override
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

    struct Timer
    {
        size_t interval = 0;

        size_t currentTime = 0;

        bool running = false;

        CallablePtr callback = nullptr;
    };

    struct TimerSystem : public System<Own<Timer>, Listener<TickEvent>, NamedSystem>
    {
        virtual std::string getSystemName() const override { return "Timer System"; }

        virtual void onEvent(const TickEvent& event) override
        {
            LOG_THIS_MEMBER("Ticking System");

            currentIncrement += event.tick;
        }

        virtual void execute() override
        {
            LOG_THIS_MEMBER("Ticking System");

            // Todo here compare and exchange currentIncrement !
            if(currentIncrement == 0) return;

            auto currentIncrementLoaded = currentIncrement.exchange(0);

            const auto ecsRef = this->world();

            for(const auto& timer : view<Timer>())
            {
                if(timer->running)
                {
                    timer->currentTime += currentIncrementLoaded;

                    while(timer->currentTime >= timer->interval)
                    {
                        timer->currentTime -= timer->interval;

                        if(timer->callback)
                            timer->callback->call(ecsRef);
                    }
                }
            }

        }

        std::atomic<size_t> currentIncrement{0};
    };

    struct TextInputTriggeredEvent
    {
        TextInputTriggeredEvent(EntityRef entity) : entity(entity) {}
        TextInputTriggeredEvent(const TextInputTriggeredEvent& other) : entity(other.entity) {}
        ~TextInputTriggeredEvent() {}

        EntityRef entity;
    };

    struct RunScriptFromTextInputSystem : public System<Listener<TextInputTriggeredEvent>, NamedSystem, StoragePolicy>
    {
        virtual std::string getSystemName() const override { return "Run Script From Text Input System"; }

        virtual void onEvent(const TextInputTriggeredEvent& event) override
        {
            LOG_THIS_MEMBER("Run Script From Text Input System");

            if(not event.entity->has<TextInputComponent>())
            {
                LOG_ERROR("Run Script From Text Input System", "Entity has no Text Input Component");
            }

            auto textComp = event.entity->get<TextInputComponent>();

            LOG_INFO("Run Script From Text Input System", "Trying to execute script: " << textComp->returnText);

            ecsRef->sendEvent(ExecuteFileScriptEvent{textComp->returnText});
        }
    };
}