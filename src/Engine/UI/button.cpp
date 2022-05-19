#include "button.h"

#include <functional>

namespace pg
{
    namespace
    {
        const char * DOM = "Button";

        template <typename Type, typename... Args>
        const InputSystem::MouseComponent& makeButtonMouseComponent(UiComponent* uiComponent, Type *obj, void(Type::*onPress)(Input*, double...), const Args&... args)
        {
            std::function<void(Input*, double)> press = [=](Input* inputHandler, double deltaTime) {
                static bool pressed = false;
                
                if(inputHandler->isButtonPressed(Qt::LeftButton))
                    pressed = true;

                if(not inputHandler->isButtonPressed(Qt::LeftButton) and pressed)
                    obj->onPress(inputHandler, deltaTime, args...);

                pressed = false;
            };

            return makeMouseArea(uiComponent, press);
        }

        const InputSystem::MouseComponent& makeButtonMouseComponent(UiComponent* uiComponent, void(*onPress)(Input*, double))
        {
            std::function<void(Input*, double)> press = [onPress](Input* inputHandler, double deltaTime) {
                static bool pressed = false;
                
                if(inputHandler->isButtonPressed(Qt::LeftButton))
                    pressed = true;

                if(not inputHandler->isButtonPressed(Qt::LeftButton) and pressed)
                    onPress(inputHandler, deltaTime);

                pressed = false;
            };

            return makeMouseArea(uiComponent, press);
        }

    }

    template<>
    void renderer(MasterRenderer* masterRenderer, Button* button)
    {
        if(button->background != nullptr)
            masterRenderer->render(button->background);

        if(button->sentence != nullptr)
            masterRenderer->render(button->sentence);
    }

    Button::Button(const InputSystem::MouseComponent& onPress, const UiComponent& frame, TextureComponent* background, Sentence* sentence) : UiComponent(frame), background(background), sentence(sentence), onPress(onPress)
    {
        if(background != nullptr)
        {
            background->setTopAnchor(this->top);
            background->setLeftAnchor(this->left);
        }
            
        if(sentence != nullptr)
        {
            sentence->setTopAnchor(this->top);
            sentence->setLeftAnchor(this->left);
        }
    }
        
    template<typename Type, typename... Args>
    Button::Button(const Type& object, void(Type::*onPress)(Input*, double), const UiComponent& frame, TextureComponent* background, Sentence* sentence, const Args&... args) : Button(makeButtonMouseComponent(this, object, onPress, args...), frame, background, sentence)
    {

    }
    
    Button::Button(void(*onPress)(Input*, double), const UiComponent& frame, TextureComponent* background, Sentence* sentence) : Button(makeButtonMouseComponent(this, onPress), frame, background, sentence)
    {
    
    }

    Button::~Button()
    {
        deleteInput(onPress);
    }

    void Button::show()
    {
        if(background != nullptr)
            background->show();

        if(sentence != nullptr)
            sentence->show();
    }

    void Button::hide()
    {
        if(background != nullptr)
            background->hide();

        if(sentence != nullptr)
            sentence->hide();
    }
}