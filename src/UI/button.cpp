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

    Button::Button(const InputSystem::MouseComponent& onPress, const UiComponent& frame, TextureComponent* background, Sentence* sentence)
    {
    }
        
    template<typename Type, typename... Args>
    Button::Button(const Type& object, void(Type::*onPress)(Input*, double), const UiComponent& frame, TextureComponent* background, Sentence* sentence) : Button(makeButtonMouseComponent(this, onPress), frame, background, sentence)
    {

    }
    
    Button::Button(void(*onPress)(Input*, double), const UiComponent& frame, TextureComponent* background, Sentence* sentence) : Button(makeButtonMouseComponent(this, onPress), frame, background, sentence)
    {
    
    }

    Button::~Button()
    {

    }
}