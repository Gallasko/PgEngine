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

    Button::Button(const InputSystem::MouseComponent& onPress, TextureComponent* background, Sentence* sentence, const UiComponent& frame) : UiComponent(frame), background(background), sentence(sentence), onPress(onPress)
    {
        moveUiElements();
    }
        
    template<typename Type, typename... Args>
    Button::Button(const Type& object, void(Type::*onPress)(Input*, double), TextureComponent* background, Sentence* sentence, const UiComponent& frame, const Args&... args) : UiComponent(frame), background(background), sentence(sentence), onPress(makeButtonMouseComponent(this, object, onPress, args...))
    {
        moveUiElements();
    }
    
    Button::Button(void(*onPress)(Input*, double), TextureComponent* background, Sentence* sentence, const UiComponent& frame) : UiComponent(frame), background(background), sentence(sentence), onPress(makeButtonMouseComponent(this, onPress))
    {
        moveUiElements();
    }

    Button::Button(void(*onPress)(Input*, double), const Sentence::SentenceParameters& sentence, const UiComponent& frame) : UiComponent(frame), onPress(makeButtonMouseComponent(this, onPress))
    {
        this->sentence = new Sentence(sentence);
        ownSentence = true;

        this->sentence->setTopAnchor(this->top);
        this->sentence->setLeftAnchor(this->left);

        // Align text at the center of the button
        this->sentence->setLeftMargin(this->width / 2.0f - this->sentence->width / 2.0f);
        this->sentence->setTopMargin(this->height / 2.0f - this->sentence->height / 2.0f);
    }

    Button::Button(void(*onPress)(Input*, double), const std::string& textureName, const UiComponent& frame) : UiComponent(frame), onPress(makeButtonMouseComponent(this, onPress))
    {
        this->background = new TextureComponent(this->width, this->height, textureName);
        ownBackground = true;

    }

    Button::Button(void(*onPress)(Input*, double), const std::string& textureName, const Sentence::SentenceParameters& sentence, const UiComponent& frame) : UiComponent(frame), onPress(makeButtonMouseComponent(this, onPress))
    {
        this->sentence = new Sentence(sentence);
        ownSentence = true;

        this->background = new TextureComponent(this->width, this->height, textureName);
        ownBackground = true;
    }

    Button::Button(const Button& rhs) : UiComponent(rhs), onPress(makeMouseArea(this, rhs.onPress)), ownBackground(rhs.ownBackground), ownSentence(rhs.ownSentence)
    {
        if(rhs.ownBackground)
        {
            this->background = new TextureComponent(this->width, this->height, rhs.background->textureName);
        }
        else
            this->background = rhs.background;

        if(rhs.ownSentence)
        {
            this->sentence = new Sentence(rhs.sentence->text, rhs.sentence->scale, rhs.sentence->font);

            this->sentence->setTopAnchor(this->top);
            this->sentence->setLeftAnchor(this->left);

            this->sentence->setLeftMargin(this->width / 2.0f - this->sentence->width / 2.0f);
            this->sentence->setTopMargin(this->height / 2.0f - this->sentence->height / 2.0f);
        }
        else
            this->sentence = rhs.sentence;
    }

    Button::~Button()
    {
        deleteInput(onPress);

        if(ownSentence)
            delete sentence;

        if(ownBackground)
            delete background;
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

    void Button::moveUiElements()
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
}