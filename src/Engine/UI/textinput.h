#pragma once

#include "ECS/entitysystem.h"

#include "Input/inputcomponent.h"

#include "focusable.h"

namespace pg
{
    struct TextInputComponent
    {
        TextInputComponent(std::shared_ptr<AbstractCallable> callback, const std::string& defaultText = "") : callback(callback), text(defaultText) { LOG_THIS_MEMBER("TextInputComponent"); }
        TextInputComponent(const TextInputComponent& rhs) : callback(rhs.callback) { LOG_THIS_MEMBER("TextInputComponent"); }
        virtual ~TextInputComponent() { LOG_THIS_MEMBER("TextInputComponent"); }

        std::shared_ptr<AbstractCallable> callback;
        
        std::string text;
    };

    struct TextInputSystem: public System<Own<TextInputComponent>, Ref<FocusableComponent>, Listener<OnSDLTextInput>, Listener<OnSDLScanCode>, NamedSystem, InitSys, StoragePolicy>
    {
        virtual void init() override
        {
            registerGroup<TextInputComponent, FocusableComponent>();
        }

        virtual std::string getSystemName() const override { return "Text Input System"; }

        virtual void onEvent(const OnSDLTextInput& event) override;

        virtual void onEvent(const OnSDLScanCode& event) override;
    };
}