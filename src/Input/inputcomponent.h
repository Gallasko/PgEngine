#pragma once

#include "../UI/uisystem.h"
#include "input.h"

#include <memory>

namespace pg
{
    struct MouseInputComponent
    {
        struct Base {};

        // TODO replace this by a UiFrame
        UiPosition *pos;
        UiSize *width, *height; // Input Area
        const bool *enable;

        Base *object;

        void (Base::*onPressed)(Input*, double, ...) = nullptr;
        void (Base::*onLeave)(Input*, double) = nullptr;

        void (*onPressedLambda)(Input*, double) = nullptr;
        void (*onLeaveLambda)(Input*, double) = nullptr;

        template<typename Func>
        void registerFunc(void (Func::*mouseInput)(Input*, double, ...), Func *obj, void (Func::*mouseLeave)(Input*, double) = nullptr)
        { 
            struct Delegate : public Func, public Base {};

            onPressed = static_cast<void (Base::*)(Input*, double, ...)>( static_cast<void (Delegate::*)(Input*, double, ...)>(mouseInput) );
            onLeave = static_cast<void (Base::*)(Input*, double)>( static_cast<void (Delegate::*)(Input*, double)>(mouseLeave) );

            object = static_cast<Delegate* >(obj);
        }

        void registerFunc(void (*mouseEnter)(Input*, double), void (*mouseLeave)(Input*, double))
        {
            onPressedLambda = mouseEnter;
            onLeaveLambda = mouseLeave;
        }

        MouseInputComponent(UiComponent *component) : pos(&component->pos), width(&component->width), height(&component->height), enable(&component->isVisible()) {}
        MouseInputComponent(const MouseInputComponent& component) : pos(component.pos), width(component.width), height(component.height), enable(component.enable), object(component.object), onPressed(component.onPressed), onLeave(component.onLeave), onPressedLambda(component.onPressedLambda), onLeaveLambda(component.onLeaveLambda) {}

        //TODO check if we can send const Args& all the time or if some specific application need to modify arguments
        template<typename... Args>
        void call(Input* inputHandler, double deltaTime, const Args&... args)
        {
            if(onPressed != nullptr)
                (*object.*onPressed)(inputHandler, deltaTime, args...);
            
            if(onPressedLambda != nullptr) 
                (*onPressedLambda)(inputHandler, deltaTime);
        }

        // TODO see if leave function need more args or not
        void leave(Input* inputHandler, double deltaTime)
        {
            if(onLeave != nullptr)
                (*object.*onLeave)(inputHandler, deltaTime);

            if(onLeaveLambda != nullptr) 
                (*onLeaveLambda)(inputHandler, deltaTime);
        }

        bool inBound(int x, int y) const { return x > this->pos->x && x < (this->pos->x + *this->width) && y < (this->pos->y + *this->height) && y > this->pos->y; }
        bool inBound(const constant::Vector2D& vec2) const { return inBound(vec2.x, vec2.y); }

        virtual ~MouseInputComponent() {}
    };

    template<typename ObjectType>
    struct MouseInputBase : public MouseInputComponent
    {
        ObjectType *object;

        void (ObjectType::*onPressed)(Input*, double, ...) = nullptr;
        void (ObjectType::*onLeave)(Input*, double) = nullptr;

        using MouseInputComponent::MouseInputComponent;

        template<typename... Args>
        void call(Input* inputHandler, double deltaTime, const Args&... args)
        {
            if(onPressed != nullptr)
            {
                auto obj = static_cast<ObjectType*>(object);
                auto f = static_cast<void (ObjectType::*)(Input*, double, ...)>(*onPressed);

                (obj->f)(inputHandler, deltaTime, args...);
            } 

            if(onPressedLambda != nullptr)
                (*onPressedLambda)(inputHandler, deltaTime);
        }

        // TODO see if leave function need more args or not
        void leave(Input* inputHandler, double deltaTime)
        {
            if(onLeave != nullptr)
            {
                auto obj = static_cast<ObjectType*>(object);
                auto f = static_cast<void (ObjectType::*)(Input*, double)>(*onLeave);

                (obj->f)(inputHandler, deltaTime);
            } 

            if(onLeaveLambda != nullptr) 
                (*onLeaveLambda)(inputHandler, deltaTime);
        }

        ~MouseInputBase() {}
    };

    struct KeyboardInputComponent
    {
        struct Base {};

        Base *object;

        void (Base::*onKey)(Input*, double, ...) = nullptr;

        void (*onKeyLambda)(Input*, double) = nullptr;

        KeyboardInputComponent() {}
        KeyboardInputComponent(const KeyboardInputComponent& component) : object(component.object), onKey(component.onKey), onKeyLambda(component.onKeyLambda) {}

        template<typename Func>
        void registerFunc(void (Func::*f)(Input*, double, ...), Func *obj)
        { 
            struct Delegate : public Func, public Base {};

            onKey = static_cast<void (Base::*)(Input*, double, ...)>( static_cast<void (Delegate::*)(Input*, double, ...)>(f) );
            object = static_cast<Delegate* >(obj);
        }

        void registerFunc(void (*f)(Input*, double)) { onKeyLambda = f; }

        template<typename... Args>
        void call(Input* inputHandler, double deltaTime, const Args&... args) { if(onKey != nullptr) (*object.*onKey)(inputHandler, deltaTime, args...); if(onKeyLambda != nullptr) (*onKeyLambda)(inputHandler, deltaTime); }

        virtual ~KeyboardInputComponent() {}
    };

    template<typename ObjectType>
    struct KeyboardInputBase : public KeyboardInputComponent
    {
        ObjectType *object;

        void (ObjectType::*onKey)(Input*, double, ...) = nullptr;

        using KeyboardInputComponent::KeyboardInputComponent;

        template<typename... Args>
        void call(Input* inputHandler, double deltaTime, const Args&... args) { if(onKey != nullptr) {auto obj = static_cast<ObjectType*>(object); auto f = static_cast<void (ObjectType::*)(Input*, double, ...)>(*onKey); (obj->f)(inputHandler, deltaTime, args...);} if(onKeyLambda != nullptr) (*onKeyLambda)(inputHandler, deltaTime);  }

        ~KeyboardInputBase() {}
    };

    typedef std::shared_ptr<MouseInputComponent> MouseInputPtr;
    typedef std::shared_ptr<KeyboardInputComponent> KeyInputPtr;

    class InputSystem 
    {
        // Typedefs

        /** InputSystem unique pointer type definition */
        typedef std::unique_ptr<InputSystem> InputPtr;

    public:
        // Helper Struct
        struct MouseComponent
        {
            MouseComponent(const MouseInputPtr& component, const std::function<void(Input*, double)>& inputCallback, const std::function<void(Input*, double)>& leaveCallback = nullptr) : component(component), inputCallback(inputCallback), leaveCallback(leaveCallback) {}
            
            bool operator==(const MouseComponent& rhs) const { return component == rhs.component; }

            MouseInputPtr component;
            std::function<void(Input*, double)> inputCallback;
            std::function<void(Input*, double)> leaveCallback;
        };

        // Helper Struct
        struct KeyComponent
        {
            KeyComponent(const KeyInputPtr& component, const std::function<void(Input*, double)>& callback) : component(component), callback(callback) {}
            
            bool operator==(const KeyComponent& rhs) const { return component == rhs.component; }

            KeyInputPtr component;
            std::function<void(Input*, double)> callback;
        };

    public:
        static const InputPtr& system() { static auto system = InputPtr(new InputSystem()); return system; }

        void deleteInput(const InputSystem::MouseComponent& component);
        void deleteInput(const InputSystem::KeyComponent& component);

        void updateState(Input* inputHandler, double deltaTime);

        template<typename ObjectType, typename... Args>
        friend const InputSystem::MouseComponent& makeMouseArea(UiComponent *component, ObjectType *obj, void (ObjectType::*mouseInput)(Input*, double, ...), void (ObjectType::*mouseLeave)(Input*, double) = nullptr, const Args&... args);

        template<typename ObjectType, typename... Args>
        friend const InputSystem::MouseComponent& makeMouseArea(UiComponent *component, ObjectType *obj, void (ObjectType::*mouseInput)(Input*, double, ...), std::nullptr_t, const Args&... args);
    
        template<typename... Args>
        friend const InputSystem::MouseComponent& makeMouseArea(UiComponent *component, void (*mouseInput)(Input*, double), void (*mouseLeave)(Input*, double) = nullptr, const Args&... args);

        template<typename... Args>
        friend const InputSystem::MouseComponent& makeMouseArea(UiComponent *component, void (*mouseInput)(Input*, double), void (*mouseLeave)(Input*, double), const Args&... args);
    
        template<typename ObjectType, typename... Args>
        friend const InputSystem::KeyComponent& makeKeyInput(ObjectType *obj, void (ObjectType::*f)(Input*, double, ...), const Args&... args);

        template<typename... Args>
        friend const InputSystem::KeyComponent& makeKeyInput(void (*f)(Input*, double), const Args&... args);

    private:
        const InputSystem::MouseComponent& registerMouseArea(MouseInputPtr component, const std::function<void(Input*, double)>& inputCallback, const std::function<void(Input*, double)>& leaveCallback = nullptr);
        const InputSystem::KeyComponent& registerKeyInput(KeyInputPtr component, const std::function<void(Input*, double)>& callback);
        
        std::vector<InputSystem::MouseComponent> mouseComponents;
        std::vector<InputSystem::KeyComponent> keyComponents;
    };

    template<typename ObjectType, typename... Args>
    const InputSystem::MouseComponent& makeMouseArea(UiComponent *component, ObjectType *obj, void (ObjectType::*mouseInput)(Input*, double, ...), void (ObjectType::*mouseLeave)(Input*, double), const Args&... args)
    {
        auto& system = InputSystem::system();

        auto mouseArea = MouseInputPtr(new MouseInputBase<ObjectType>(component));
        mouseArea->registerFunc(mouseInput, obj, mouseLeave);

        auto inputCallback = [=](Input* inputHandler, double deltaTime) { mouseArea->call(inputHandler, deltaTime, args...); };

        std::function<void(Input*, double)> leaveCallback;
        if(mouseLeave != nullptr)
            leaveCallback = [=](Input* inputHandler, double deltaTime) { mouseArea->leave(inputHandler, deltaTime); };
        else
            leaveCallback = nullptr;

        return system->registerMouseArea(mouseArea, inputCallback, leaveCallback);
    }

    template<typename ObjectType, typename... Args>
    const InputSystem::MouseComponent& makeMouseArea(UiComponent *component, ObjectType *obj, void (ObjectType::*mouseInput)(Input*, double, ...), std::nullptr_t, const Args&... args)
    {
        auto& system = InputSystem::system();

        auto mouseArea = MouseInputPtr(new MouseInputBase<ObjectType>(component));
        mouseArea->registerFunc(mouseInput, obj, static_cast<void (ObjectType::*)(pg::Input*, double)>(nullptr));

        auto inputCallback = [=](Input* inputHandler, double deltaTime) { mouseArea->call(inputHandler, deltaTime, args...); };

        return system->registerMouseArea(mouseArea, inputCallback, static_cast<std::function<void(pg::Input*, double)>>(nullptr));
    }

    template<typename... Args>
    const InputSystem::MouseComponent& makeMouseArea(UiComponent *component, void (*mouseInput)(Input*, double), void (*mouseLeave)(Input*, double), const Args&... args)
    {
        auto& system = InputSystem::system();

        auto mouseArea = MouseInputPtr(new MouseInputBase<MouseInputComponent::Base>(component));
        mouseArea->registerFunc(mouseInput, mouseLeave);

        auto inputCallback = [=](Input* inputHandler, double deltaTime) { mouseArea->call(inputHandler, deltaTime, args...); };

        std::function<void(Input*, double)> leaveCallback;
        if(mouseLeave != nullptr)
            leaveCallback = [=](Input* inputHandler, double deltaTime) { mouseArea->leave(inputHandler, deltaTime); };
        else
            leaveCallback = nullptr;

        return system->registerMouseArea(mouseArea, inputCallback, leaveCallback);
    }

    template<typename... Args>
    const InputSystem::MouseComponent& makeMouseArea(UiComponent *component, void (*mouseInput)(Input*, double), std::nullptr_t, const Args&... args)
    {
        auto& system = InputSystem::system();

        auto mouseArea = MouseInputPtr(new MouseInputBase<MouseInputComponent::Base>(component));
        mouseArea->registerFunc(mouseInput, static_cast<void (*)(pg::Input*, double)>(nullptr));

        auto inputCallback = [=](Input* inputHandler, double deltaTime) { mouseArea->call(inputHandler, deltaTime, args...); };

        return system->registerMouseArea(mouseArea, inputCallback, static_cast<std::function<void(pg::Input*, double)>>(nullptr));
    }

    template<typename ObjectType, typename... Args>
    const InputSystem::KeyComponent& makeKeyInput(ObjectType *obj, void (ObjectType::*f)(Input*, double, ...), const Args&... args)
    {
        auto& system = InputSystem::system();

        auto keyInput = KeyInputPtr(new KeyboardInputBase<ObjectType>());
        keyInput->registerFunc(f, obj);

        auto callback = [=](Input* inputHandler, double deltaTime) { keyInput->call(inputHandler, deltaTime, args...); };

        return system->registerKeyInput(keyInput, callback);
    }

    template<typename... Args>
    const InputSystem::KeyComponent& makeKeyInput(void (*f)(Input*, double), const Args&... args)
    {
        auto& system = InputSystem::system();

        auto keyInput = KeyInputPtr(new KeyboardInputBase<KeyboardInputComponent::Base>());
        keyInput->registerFunc(f);

        auto callback = [=](Input* inputHandler, double deltaTime) { keyInput->call(inputHandler, deltaTime, args...); };

        return system->registerKeyInput(keyInput, callback);
    }

    template<typename Input>
    void deleteInput(const Input& input)
    {
        auto& system = InputSystem::system();

        system->deleteInput(input);
    }
}