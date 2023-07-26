#pragma once

#include "ECS/entitysystem.h"

#include "Input/inputcomponent.h"

#include "focusable.h"

namespace pg
{
    struct TextInputComponent
    {
        TextInputComponent(std::shared_ptr<AbstractCallable> callback, const std::string& defaultText) : callback(callback), text(defaultText) { LOG_THIS_MEMBER("TextInputComponent"); }
        TextInputComponent(const TextInputComponent& rhs) : callback(rhs.callback) { LOG_THIS_MEMBER("TextInputComponent"); }
        virtual ~TextInputComponent() { LOG_THIS_MEMBER("TextInputComponent"); }

        std::string text;

        std::shared_ptr<AbstractCallable> callback;
    };

    struct TextInputSystem: public System<Listener<OnSDLTextInput>, Own<TextInputComponent>, Ref<FocusableComponent>>
    {
        virtual void onEvent(const OnSDLTextInput& event) override;
    };

    //struct TextInputRenderer 

}