// #include "button.h"

// #include "UI/texture.h"

// #include <functional>

// namespace pg
// {
//     namespace
//     {
//         static constexpr char const * DOM = "Button";

//         template <typename Type, typename... Args>
//         Entity* makeButtonMouseComponent(EntitySystem *ecs, UiComponent* uiComponent, Type *obj, void(Type::*onPress)(Input*, double...), const Args&... args)
//         {
//             return makeMouseArea(ecs, uiComponent, [obj, onPress, args...](Input* inputHandler, double deltaTime) {
//                 static bool pressed = false;
                
//                 if(inputHandler->isButtonPressed(Qt::LeftButton))
//                     pressed = true;

//                 if(not inputHandler->isButtonPressed(Qt::LeftButton))
//                 {
//                     if(pressed)
//                         obj->onPress(inputHandler, deltaTime, args...);

//                     pressed = false;
//                 }
//             });
//         }

//         Entity* makeButtonMouseComponent(EntitySystem *ecs, UiComponent* uiComponent, void(*onPress)(Input*, double))
//         {
//             return makeMouseArea(ecs, uiComponent, [onPress](Input* inputHandler, double deltaTime) {
//                 static bool pressed = false;
                
//                 if(inputHandler->isButtonPressed(Qt::LeftButton))
//                     pressed = true;

//                 if(not inputHandler->isButtonPressed(Qt::LeftButton))
//                 {
//                     if(pressed)
//                         onPress(inputHandler, deltaTime);

//                     pressed = false;
//                 }
//             });
//         }

//         Entity* makeButtonMouseComponent(EntitySystem *ecs, UiComponent *uiComponent, const std::function<void(Input*, double)>& onPress)
//         {
//             return makeMouseArea(ecs, uiComponent, [onPress](Input* inputHandler, double deltaTime) {
//                 static bool pressed = false;
                
//                 if(inputHandler->isButtonPressed(Qt::LeftButton))
//                     pressed = true;

//                 if(not inputHandler->isButtonPressed(Qt::LeftButton))
//                 {
//                     if(pressed)
//                         onPress(inputHandler, deltaTime);

//                     pressed = false;
//                 }
//             });
//         }
//     }

//     template <>
//     void renderer(MasterRenderer* masterRenderer, Button* button)
//     {
//         // if(button->background != nullptr)
//         //     masterRenderer->render(button->background);

//         if(button->sentence != nullptr)
//             masterRenderer->render(button->sentence);
//     }

//     // Todo fix all of this

//     // TODO create an edge case for a copy of this type of button cause it doesn t have a callback
//     Button::Button(EntitySystem *ecs, MouseComponent* onPress, Texture2DComponent* background, Sentence* sentence, const UiComponent& frame) : UiComponent(frame), background(background), sentence(sentence)
//     {
//         moveUiElements();
//     }

//     Button::Button(EntitySystem *ecs, void(*onPress)(Input*, double), Texture2DComponent* background, Sentence* sentence, const UiComponent& frame) : UiComponent(frame), background(background), sentence(sentence), callback(onPress), compoundEntity(makeButtonMouseComponent(ecs, this, callback))
//     {
//         moveUiElements();
//     }

//     Button::Button(EntitySystem *ecs, void(*onPress)(Input*, double), const Sentence::SentenceParameters& sentence, const UiComponent& frame) : UiComponent(frame), callback(onPress), compoundEntity(makeButtonMouseComponent(ecs, this, callback))
//     {
//         this->sentence = new Sentence(sentence);
//         ownSentence = true;

//         moveUiElements();

//         this->width = this->sentence->width;
//         this->height = this->sentence->height;
//     }

//     Button::Button(EntitySystem *ecs, void(*onPress)(Input*, double), const std::string& textureName, const UiComponent& frame) : UiComponent(frame), callback(onPress), compoundEntity(makeButtonMouseComponent(ecs, this, callback))
//     {
//         // this->background = new Texture2DComponent(this->width, this->height, textureName);
//         ownBackground = true;

//         moveUiElements();
//     }

//     Button::Button(EntitySystem *ecs, void(*onPress)(Input*, double), const std::string& textureName, const Sentence::SentenceParameters& sentence, const UiComponent& frame) : UiComponent(frame), callback(onPress), compoundEntity(makeButtonMouseComponent(ecs, this, callback))
//     {
//         this->sentence = new Sentence(sentence);
//         ownSentence = true;

//         this->width = this->sentence->width;
//         this->height = this->sentence->height;

//         // this->background = new Texture2DComponent(this->width, this->height, textureName);
//         ownBackground = true;

//         moveUiElements();
//     }

//     Button::Button(EntitySystem *ecs, const std::function<void(Input*, double)>& onPress, const UiComponent& frame) : UiComponent(frame), callback(onPress), compoundEntity(makeButtonMouseComponent(ecs, this, callback))
//     {

//     }

//     Button::Button(EntitySystem *ecs, const std::function<void(Input*, double)>& onPress, const std::string& textureName, const UiComponent& frame) : Button(ecs, onPress, frame)
//     {
//         // this->background = new Texture2DComponent(this->width, this->height, textureName);
//         ownBackground = true;

//         moveUiElements();
//     }

//     Button::Button(EntitySystem *ecs, const std::function<void(Input*, double)>& onPress, const Sentence::SentenceParameters& sentence, const UiComponent& frame) : Button(ecs, onPress, frame)
//     {
//         this->sentence = new Sentence(sentence);
//         ownSentence = true;

//         moveUiElements();

//         this->width = this->sentence->width;
//         this->height = this->sentence->height;
//     }

//     Button::Button(EntitySystem *ecs, const std::function<void(Input*, double)>& onPress, const std::string& textureName, const Sentence::SentenceParameters& sentence, const UiComponent& frame) : Button(ecs, onPress, frame)
//     {
//         this->sentence = new Sentence(sentence);
//         ownSentence = true;

//         this->width = this->sentence->width;
//         this->height = this->sentence->height;

//         // this->background = new Texture2DComponent(this->width, this->height, textureName);
//         ownBackground = true;

//         moveUiElements();
//     }

//     Button::Button(const Button& rhs) : UiComponent(rhs.frame), callback(rhs.callback), compoundEntity(makeButtonMouseComponent(rhs.compoundEntity->world(), this, callback)), ownBackground(rhs.ownBackground), ownSentence(rhs.ownSentence)
//     {
//         if(rhs.ownBackground)
//         {
//             // this->background = new Texture2DComponent(this->width, this->height, rhs.background->textureName);

//             moveUiElements();
//         }
//         else
//             this->background = rhs.background;

//         if(rhs.ownSentence)
//         {
//             this->sentence = new Sentence(rhs.sentence->text, rhs.sentence->scale, rhs.sentence->font);

//             moveUiElements();
//         }
//         else
//             this->sentence = rhs.sentence;
//     }

//     Button::~Button()
//     {
//         if(ownSentence)
//             delete sentence;

//         if(ownBackground)
//             delete background;
//     }

//     void Button::render(MasterRenderer *masterRenderer)
//     {
//         renderer(masterRenderer, this);
//     }

//     void Button::show()
//     {
//         UiComponent::show();

//         // if(background != nullptr)
//         //     background->show();

//         if(sentence != nullptr)
//             sentence->show();
//     }

//     void Button::hide()
//     {
//         UiComponent::hide();
        
//         // if(background != nullptr)
//         //     background->hide();

//         if(sentence != nullptr)
//             sentence->hide();
//     }

//     void Button::moveUiElements()
//     {
//         // if(background != nullptr)
//         // {
//         //     background->setTopAnchor(this->top);
//         //     background->setLeftAnchor(this->left);
//         // }
            
//         if(sentence != nullptr)
//         {
//             this->sentence->setTopAnchor(this->top);
//             this->sentence->setLeftAnchor(this->left);

//             // Align text at the center of the button
//             this->sentence->setLeftMargin(this->width / 2.0f - this->sentence->width / 2.0f);
//             this->sentence->setTopMargin(this->height / 2.0f - this->sentence->height / 2.0f);
//         }
//     }
// }