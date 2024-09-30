#pragma once

#include "audiosystem.h"

#include "Interpreter/pginterpreter.h"

namespace pg
{
    class PlayAudioFunction : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem* ecsRef)
        {
            setArity(1, 2);

            this->ecsRef = ecsRef;
        }

        virtual ValuablePtr call(ValuableQueue& args) override
        {
            auto v = args.front()->getElement();
            args.pop();

            int loop = -1;

            if (not args.empty())
            {
                auto l = args.front()->getElement();

                if (not l.isNumber())
                {
                    LOG_ERROR("Play Audio Function", "Loop should be a number");
                
                    return nullptr;
                }

                loop = l.get<int>();
            }

            if (not v.isLitteral())
            {
                LOG_ERROR("Play Audio Function", "Music should be a litteral");
                
                return nullptr;
            }

            ecsRef->sendEvent(StartAudio{v.toString(), loop});

            return nullptr;
        }

        EntitySystem* ecsRef = nullptr;
    };

    class StopAudioFunction : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem* ecsRef)
        {
            setArity(0, 0);

            this->ecsRef = ecsRef;
        }

        virtual ValuablePtr call(ValuableQueue&) override
        {
            ecsRef->sendEvent(StopAudio{});

            return nullptr;
        }

        EntitySystem* ecsRef = nullptr;
    };

    class PauseAudioFunction : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem* ecsRef)
        {
            setArity(0, 0);

            this->ecsRef = ecsRef;
        }

        virtual ValuablePtr call(ValuableQueue&) override
        {
            ecsRef->sendEvent(PauseAudio{});

            return nullptr;
        }

        EntitySystem* ecsRef = nullptr;
    };

    class ResumeAudioFunction : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem* ecsRef)
        {
            setArity(0, 0);

            this->ecsRef = ecsRef;
        }

        virtual ValuablePtr call(ValuableQueue&) override
        {
            ecsRef->sendEvent(ResumeAudio{});

            return nullptr;
        }

        EntitySystem* ecsRef = nullptr;
    };

    class PlaySoundEffectFunction : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem* ecsRef)
        {
            setArity(1, 3);

            this->ecsRef = ecsRef;
        }

        virtual ValuablePtr call(ValuableQueue& args) override
        {
            auto v = args.front()->getElement();
            args.pop();

            int loop = 0;

            int channel = -1;

            if (not v.isLitteral())
            {
                LOG_ERROR("Play Sound Effect Function", "Music should be a litteral");
                
                return nullptr;
            }

            if (not args.empty())
            {
                auto l = args.front()->getElement();
                args.pop();

                if (not l.isNumber())
                {
                    LOG_ERROR("Play Sound Effect Function", "Loop should be a number");
                
                    return nullptr;
                }

                loop = l.get<int>();
            }

            if (not args.empty())
            {
                auto c = args.front()->getElement();
                args.pop();

                if (not c.isNumber())
                {
                    LOG_ERROR("Play Sound Effect Function", "Channel should be a number");
                
                    return nullptr;
                }

                channel = c.get<int>();
            }

            ecsRef->sendEvent(PlaySoundEffect{v.toString(), loop, channel});

            return nullptr;
        }

        EntitySystem* ecsRef = nullptr;
    };

    struct AudioModule : public SysModule
    {
        AudioModule(EntitySystem *ecsRef)
        {
            addSystemFunction<PlayAudioFunction>("playAudio", ecsRef);
            addSystemFunction<StopAudioFunction>("stopAudio", ecsRef);
            addSystemFunction<PauseAudioFunction>("pauseAudio", ecsRef);
            addSystemFunction<ResumeAudioFunction>("resumeAudio", ecsRef);
            addSystemFunction<PlaySoundEffectFunction>("playSoundEffect", ecsRef);
        }
    };
}