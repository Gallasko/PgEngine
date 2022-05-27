#pragma once

#include "uisystem.h"
#include "sentencesystem.h"

#include "../Input/inputcomponent.h"

namespace pg
{
    class Button : public UiComponent
    {
    public:
        Button(const InputSystem::MouseComponent& onPress, TextureComponent* background = nullptr, Sentence* sentence = nullptr, const UiComponent& frame = UiComponent());
        
        template<typename Type, typename... Args>
        Button(const Type& object, void(Type::*onPress)(Input*, double), TextureComponent* background = nullptr, Sentence* sentence = nullptr, const UiComponent& frame = UiComponent(), const Args&... args);
        
        Button(void(*onPress)(Input*, double), TextureComponent* background = nullptr, Sentence* sentence = nullptr, const UiComponent& frame = UiComponent());

        Button(void(*onPress)(Input*, double), const Sentence::SentenceParameters& sentence, const UiComponent& frame = UiComponent());
        Button(void(*onPress)(Input*, double), const std::string& textureName, const UiComponent& frame = UiComponent());
        Button(void(*onPress)(Input*, double), const std::string& textureName, const Sentence::SentenceParameters& sentence, const UiComponent& frame = UiComponent());

        Button(const Button& rhs);

        ~Button();

        void show() override;
        void hide() override;
        
    public:
        TextureComponent* background = nullptr;
        Sentence* sentence = nullptr;

    private:
        void moveUiElements();

        //TODO make sure to copy that when making a copy of this button
        InputSystem::MouseComponent onPress;

        bool ownBackground = false;
        bool ownSentence = false;
    };
}