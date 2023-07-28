#include "textinput.h"

#include "sentencesystem.h"

#include "logger.h"

namespace pg
{
    namespace
    {
        static constexpr char const * DOM = "Text Input System";
    }

    void TextInputSystem::onEvent(const OnSDLTextInput& event)
    {
        for(const auto& entity : viewGroup<TextInputComponent, FocusableComponent>())
        {
            auto focus = entity->get<FocusableComponent>();

            if(focus->focused)
            {
                auto text = entity->get<TextInputComponent>();

                text->text += event.text;

                if(entity->entity->has<SentenceText>())
                {
                    world()->sendEvent(OnTextChanged{entity->entity->id, text->text});
                }

                LOG_INFO(DOM, "Current Text: " << text->text);
            }
        }
    }

    void TextInputSystem::onEvent(const OnSDLScanCode& event)
    {
        if(event.key != SDL_SCANCODE_RETURN)
            return;

        for(const auto& entity : viewGroup<TextInputComponent, FocusableComponent>())
        {
            auto focus = entity->get<FocusableComponent>();

            if(focus->focused)
            {
                auto text = entity->get<TextInputComponent>();

                text->callback->call(world());

                text->text = "";

                if(entity->entity->has<SentenceText>())
                {
                    world()->sendEvent(OnTextChanged{entity->entity->id, text->text});
                }
            }
        }
    }

}