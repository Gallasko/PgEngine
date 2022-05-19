#pragma once

#include "uisystem.h"
#include "sentencesystem.h"

#include "../Input/inputcomponent.h"

namespace pg
{
    class Button : public UiComponent
    {
    public:
        Button(const InputSystem::MouseComponent& onPress, const UiComponent& frame = UiComponent(), TextureComponent* background = nullptr, Sentence* sentence = nullptr);
        
        template<typename Type, typename... Args>
        Button(const Type& object, void(Type::*onPress)(Input*, double), const UiComponent& frame = UiComponent(), TextureComponent* background = nullptr, Sentence* sentence = nullptr, const Args&... args);
        
        Button(void(*onPress)(Input*, double), const UiComponent& frame = UiComponent(), TextureComponent* background = nullptr, Sentence* sentence = nullptr);

        ~Button();

        void show() override;
        void hide() override;
        
    public:
        TextureComponent* background;
        Sentence* sentence;

    private:        
        InputSystem::MouseComponent onPress;
    };
}