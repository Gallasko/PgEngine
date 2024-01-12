#pragma once

#include "ECS/system.h"

namespace pg
{
    struct StartAudio {};
    struct StopAudio {};
    struct PauseAudio {};
    struct ResumeAudio {};

    struct PlaySoundEffect {};

    struct AudioSystem : public System<Listener<StartAudio>, Listener<StopAudio>, Listener<PauseAudio>, Listener<ResumeAudio>, Listener<PlaySoundEffect>, InitSys, NamedSystem>
    {
        virtual void init() override {}

        void closeSDLMixer();

        virtual void onEvent(const StartAudio& event) override {}
        virtual void onEvent(const StopAudio& event) override {}
        virtual void onEvent(const PauseAudio& event) override {}
        virtual void onEvent(const ResumeAudio& event) override {}

        virtual void onEvent(const PlaySoundEffect& event) override {}

        virtual std::string getSystemName() const override { return "ListView System"; }

        Mix_Music* music = nullptr;
    };
}