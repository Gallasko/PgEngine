#pragma once

#include "uisystem.h"
#include "sentencesystem.h"

#include "Input/inputcomponent.h"

namespace pg
{
    class Button : public UiComponent
    {
    public:
        Button(MouseInput onPress, TextureComponent* background = nullptr, Sentence* sentence = nullptr, const UiComponent& frame = UiComponent());
        
        template<typename Type, typename... Args>
        Button(const Type& object, void(Type::*onPress)(Input*, double), TextureComponent* background = nullptr, Sentence* sentence = nullptr, const UiComponent& frame = UiComponent(), const Args&... args);
        
        // Constructor for funtion pointer
        Button(void(*onPress)(Input*, double), TextureComponent* background = nullptr, Sentence* sentence = nullptr, const UiComponent& frame = UiComponent());
        Button(void(*onPress)(Input*, double), const Sentence::SentenceParameters& sentence, const UiComponent& frame = UiComponent());
        Button(void(*onPress)(Input*, double), const std::string& textureName, const UiComponent& frame = UiComponent());
        Button(void(*onPress)(Input*, double), const std::string& textureName, const Sentence::SentenceParameters& sentence, const UiComponent& frame = UiComponent());

        // Constructor for std::function
        Button(const std::function<void(Input*, double)>& onPress, const UiComponent& frame = UiComponent());
        Button(const std::function<void(Input*, double)>& onPress, const std::string& textureName, const UiComponent& frame = UiComponent());
        Button(const std::function<void(Input*, double)>& onPress, const Sentence::SentenceParameters& sentence, const UiComponent& frame = UiComponent());
        Button(const std::function<void(Input*, double)>& onPress, const std::string& textureName, const Sentence::SentenceParameters& sentence, const UiComponent& frame = UiComponent());

        Button(const Button& rhs);

        virtual ~Button();

        virtual void render(MasterRenderer* masterRenderer);

        // TODO
        // void setFunction(void(*onPress)(Input*, double));
        // void setFunction(const std::function<void(Input*, double)>& onPress);

        void show() override;
        void hide() override;
        
    public:
        TextureComponent* background = nullptr;
        Sentence* sentence = nullptr;

    private:
        void moveUiElements();

        std::function<void(Input*, double)> callback = nullptr;

        //TODO make sure to copy that when making a copy of this button
        MouseInput onPress;

        bool ownBackground = false;
        bool ownSentence = false;
    };
}