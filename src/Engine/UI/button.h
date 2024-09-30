// #pragma once

// #include "uisystem.h"
// #include "sentencesystem.h"

// #include "Input/inputcomponent.h"

// namespace pg
// {
//     class Texture2DComponent;

//     // Todo make a is relationship instead of inherit from UiComponent as it is like that with the new ECS
//     // Todo make it a named component instead of an unamed one
//     class Button : public UiComponent
//     {
//     public:
//         // Button(EntitySystem *ecs, MouseComponent* onPress, Texture2DComponent* background = nullptr, Sentence* sentence = nullptr, const UiComponent& frame = UiComponent());
        
//         template <typename Type, typename... Args>
//         Button(EntitySystem *ecs, const Type& object, void(Type::*onPress)(Input*, double), Texture2DComponent* background = nullptr, Sentence* sentence = nullptr, const UiComponent& frame = UiComponent(), const Args&... args);
        
//         // Constructor for funtion pointer
//         Button(EntitySystem *ecs, void(*onPress)(Input*, double), Texture2DComponent* background = nullptr, Sentence* sentence = nullptr, const UiComponent& frame = UiComponent());
//         Button(EntitySystem *ecs, void(*onPress)(Input*, double), const Sentence::SentenceParameters& sentence, const UiComponent& frame = UiComponent());
//         Button(EntitySystem *ecs, void(*onPress)(Input*, double), const std::string& textureName, const UiComponent& frame = UiComponent());
//         Button(EntitySystem *ecs, void(*onPress)(Input*, double), const std::string& textureName, const Sentence::SentenceParameters& sentence, const UiComponent& frame = UiComponent());

//         // Constructor for std::function
//         Button(EntitySystem *ecs, const std::function<void(Input*, double)>& onPress, const UiComponent& frame = UiComponent());
//         Button(EntitySystem *ecs, const std::function<void(Input*, double)>& onPress, const std::string& textureName, const UiComponent& frame = UiComponent());
//         Button(EntitySystem *ecs, const std::function<void(Input*, double)>& onPress, const Sentence::SentenceParameters& sentence, const UiComponent& frame = UiComponent());
//         Button(EntitySystem *ecs, const std::function<void(Input*, double)>& onPress, const std::string& textureName, const Sentence::SentenceParameters& sentence, const UiComponent& frame = UiComponent());

//         Button(const Button& rhs);

//         virtual ~Button();

//         virtual void render(MasterRenderer* masterRenderer);

//         // TODO
//         // void setFunction(void(*onPress)(Input*, double));
//         // void setFunction(const std::function<void(Input*, double)>& onPress);

//         void show() override;
//         void hide() override;
        
//     public:
//         Texture2DComponent* background = nullptr;
//         Sentence* sentence = nullptr;

//     private:
//         void moveUiElements();

//         std::function<void(Input*, double)> callback = nullptr;

//         //TODO make sure to copy that when making a copy of this button
//         Entity *compoundEntity = nullptr;

//         bool ownBackground = false;
//         bool ownSentence = false;
//     };

//     template <typename Type, typename... Args>
//     Button::Button(EntitySystem *ecs, const Type& object, void(Type::*onPress)(Input*, double), Texture2DComponent* background, Sentence* sentence, const UiComponent& frame, const Args&... args) : UiComponent(frame), background(background), sentence(sentence), compoundEntity(makeButtonMouseComponent(ecs, this, object, onPress, args...))
//     {
//         moveUiElements();
//     }

// }