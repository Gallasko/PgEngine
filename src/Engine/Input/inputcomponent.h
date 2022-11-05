#pragma once

#include "input.h"

#include "../constant.h"

#include "ECS/entitysystem.h"
#include "UI/uisystem.h"

#include <functional>
#include <memory>

namespace pg
{
    struct MouseInputComponent
    {
        struct Base {};

        // TODO replace this by a UiFrame
        UiPosition *pos; // Todo see if it can be set to const without conflit with MouseInput
        const UiSize *width, *height; // Input Area
        const bool *enable;

        Base *object;

        void (Base::*onPressed)(Input*, double, ...) = nullptr;
        void (Base::*onLeave)(Input*, double) = nullptr;

        void (*onPressedLambda)(Input*, double) = nullptr;
        void (*onLeaveLambda)(Input*, double) = nullptr;

        std::function<void(Input*, double)> onPressedFunction = nullptr;
        std::function<void(Input*, double)> onLeaveFunction = nullptr;

        template<typename Func>
        void registerFunc(void (Func::*mouseInput)(Input*, double, ...), Func *obj, void (Func::*mouseLeave)(Input*, double) = nullptr)
        { 
            struct Delegate : public Func, public Base {};

            onPressed = static_cast<void (Base::*)(Input*, double, ...)>( static_cast<void (Delegate::*)(Input*, double, ...)>(mouseInput) );
            onLeave = static_cast<void (Base::*)(Input*, double)>( static_cast<void (Delegate::*)(Input*, double)>(mouseLeave) );

            object = static_cast<Delegate* >(obj);
        }

        // Todo may need to implement the 4 possibles cases (lambda, lambda), (function, lambda), (lambda, function), (function, function)

        void registerFunc(void (*mouseEnter)(Input*, double), void (*mouseLeave)(Input*, double))
        {
            onPressedLambda = mouseEnter;
            onLeaveLambda = mouseLeave;

            onPressedFunction = nullptr;
            onLeaveFunction = nullptr;
        }

        void registerFunc(const std::function<void(Input*, double)>& mouseEnter, const std::function<void(Input*, double)>& mouseLeave)
        {
            onPressedFunction = mouseEnter;
            onLeaveFunction = mouseLeave;

            onPressedLambda = nullptr;
            onLeaveLambda = nullptr;
        }

        MouseInputComponent(UiComponent *component);
        MouseInputComponent(const MouseInputComponent& component);

        //TODO check if we can send const Args& all the time or if some specific application need to modify arguments
        template<typename... Args>
        void call(Input* inputHandler, double deltaTime, const Args&... args)
        {
            if(onPressed != nullptr)
                (*object.*onPressed)(inputHandler, deltaTime, args...);
            
            if(onPressedLambda != nullptr) 
                (*onPressedLambda)(inputHandler, deltaTime);

            if(onPressedFunction != nullptr)
                onPressedFunction(inputHandler, deltaTime);
        }

        // TODO see if leave function need more args or not
        void leave(Input* inputHandler, double deltaTime)
        {
            if(onLeave != nullptr)
                (*object.*onLeave)(inputHandler, deltaTime);

            if(onLeaveLambda != nullptr) 
                (*onLeaveLambda)(inputHandler, deltaTime);

            if(onLeaveFunction != nullptr)
                onLeaveFunction(inputHandler, deltaTime);
        }

        bool inBound(int x, int y) const;
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

            if(onPressedFunction != nullptr)
                onPressedFunction(inputHandler, deltaTime);
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

            if(onLeaveFunction != nullptr)
                onLeaveFunction(inputHandler, deltaTime);
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

    // Helper Struct
    struct MouseComponent : public ecs::NamedComponent<MouseComponent>
    {
        MouseComponent(const MouseInputPtr& component) : ecs::NamedComponent<MouseComponent>("MouseComponent"), component(component) {}
        
        bool operator==(const MouseComponent& rhs) const { return component == rhs.component; }

        MouseInputPtr component;
    };

    // Helper Struct
    struct KeyComponent : public ecs::NamedComponent<KeyComponent>
    {
        KeyComponent(const KeyInputPtr& component) : ecs::NamedComponent<KeyComponent>("KeyComponent"), component(component) {}
        
        bool operator==(const KeyComponent& rhs) const { return component == rhs.component; }

        KeyInputPtr component;
    };

    class InputSystem : public ecs::System<ecs::Own<MouseComponent>, ecs::Own<KeyComponent>>
    {
        /** InputSystem unique pointer type definition */
        typedef std::unique_ptr<InputSystem> InputPtr;

    public:
        InputSystem() : ecs::System<ecs::Own<MouseComponent>, ecs::Own<KeyComponent>>()
        {
            setPolicy(ecs::ExecutionPolicy::Storage);
        }

        void execute() override
        {
            for(const auto& mouse : view<MouseComponent>())
            {
                // mouse->component->call(inputHandler, deltaTime);
            }
        }

        template<typename ObjectType, typename... Args>
        MouseComponent* makeMouseArea(UiComponent *component, ObjectType *obj, void (ObjectType::*mouseInput)(Input*, double, ...), void (ObjectType::*mouseLeave)(Input*, double), ecs::EntitySystem *ecs);

        template<typename ObjectType, typename... Args>
        MouseComponent* makeMouseArea(UiComponent *component, ObjectType *obj, void (ObjectType::*mouseInput)(Input*, double, ...), std::nullptr_t, ecs::EntitySystem *ecs);

        template<typename ObjectType, typename... Args>
        MouseComponent* makeMouseArea(UiComponent *component, ObjectType *obj, void (ObjectType::*mouseInput)(Input*, double, ...), ecs::EntitySystem *ecs);
    
        friend MouseComponent* makeMouseArea(UiComponent *component, void (*mouseInput)(Input*, double), void (*mouseLeave)(Input*, double), ecs::EntitySystem *ecs);
        friend MouseComponent* makeMouseArea(UiComponent *component, void (*mouseInput)(Input*, double), std::nullptr_t, ecs::EntitySystem *ecs);
        friend MouseComponent* makeMouseArea(UiComponent *component, void (*mouseInput)(Input*, double), ecs::EntitySystem *ecs);
        friend MouseComponent* makeMouseArea(UiComponent *component, const std::function<void (Input*, double)>& mouseInput, const std::function<void (Input*, double)>& mouseLeave, ecs::EntitySystem *ecs);
        friend MouseComponent* makeMouseArea(UiComponent *component, const std::function<void (Input*, double)>& mouseInput, std::nullptr_t, ecs::EntitySystem *ecs);
        friend MouseComponent* makeMouseArea(UiComponent *component, const std::function<void (Input*, double)>& mouseInput, ecs::EntitySystem *ecs);

        template<typename ObjectType, typename... Args>
        friend KeyComponent* makeKeyInput(ObjectType *obj, void (ObjectType::*f)(Input*, double, ...), const Args&... args);

        template<typename... Args>
        friend KeyComponent* makeKeyInput(void (*f)(Input*, double), const Args&... args);

    private:
        void reorderMouse();
    };

    MouseComponent* makeMouseArea(UiComponent *component, void (*mouseInput)(Input*, double), void (*mouseLeave)(Input*, double), ecs::EntitySystem *ecs = nullptr);
    MouseComponent* makeMouseArea(UiComponent *component, void (*mouseInput)(Input*, double), std::nullptr_t, ecs::EntitySystem *ecs = nullptr);
    MouseComponent* makeMouseArea(UiComponent *component, void (*mouseInput)(Input*, double), ecs::EntitySystem *ecs = nullptr);
    MouseComponent* makeMouseArea(UiComponent *component, const std::function<void (Input*, double)>& mouseInput, const std::function<void (Input*, double)>& mouseLeave, ecs::EntitySystem *ecs = nullptr);
    MouseComponent* makeMouseArea(UiComponent *component, const std::function<void (Input*, double)>& mouseInput, std::nullptr_t, ecs::EntitySystem *ecs = nullptr);
    MouseComponent* makeMouseArea(UiComponent *component, const std::function<void (Input*, double)>& mouseInput, ecs::EntitySystem *ecs = nullptr);

    template<typename ObjectType, typename... Args>
    MouseComponent* makeMouseArea(UiComponent *component, ObjectType *obj, void (ObjectType::*mouseInput)(Input*, double, ...), void (ObjectType::*mouseLeave)(Input*, double), ecs::EntitySystem *ecs = nullptr)
    {
        auto mouseArea = MouseInputPtr(new MouseInputBase<ObjectType>(component));
        mouseArea->registerFunc(mouseInput, obj, mouseLeave);

        if(ecs)
            return ecs->attach<MouseComponent>(component, mouseArea);

        return component->world()->attach<MouseComponent>(component, mouseArea);
    }

    template<typename ObjectType, typename... Args>
    MouseComponent* makeMouseArea(UiComponent *component, ObjectType *obj, void (ObjectType::*mouseInput)(Input*, double, ...), std::nullptr_t, ecs::EntitySystem *ecs = nullptr)
    {
        auto mouseArea = MouseInputPtr(new MouseInputBase<ObjectType>(component));
        mouseArea->registerFunc(mouseInput, obj, static_cast<void (ObjectType::*)(pg::Input*, double)>(nullptr));

        if(ecs)
            return ecs->attach<MouseComponent>(component, mouseArea);

        return component->world()->attach<MouseComponent>(component, mouseArea);
    }

    template<typename ObjectType>
    MouseComponent* makeMouseArea(UiComponent *component, ObjectType *obj, void (ObjectType::*mouseInput)(Input*, double, ...), ecs::EntitySystem *ecs = nullptr)
    {
        return makeMouseArea(component, obj, mouseInput, nullptr, ecs);
    }

    template<typename ObjectType, typename... Args>
    KeyComponent* makeKeyInput(ObjectType *obj, void (ObjectType::*f)(Input*, double, ...), const Args&... args)
    {
        auto keyInput = KeyInputPtr(new KeyboardInputBase<ObjectType>());
        keyInput->registerFunc(f, obj);

        // TODO
        return nullptr;

        // return ecs->attach<KeyComponent>(obj, keyInput);
    }

    template<typename... Args>
    KeyComponent* makeKeyInput(void (*f)(Input*, double), const Args&... args)
    {
        auto keyInput = KeyInputPtr(new KeyboardInputBase<KeyboardInputComponent::Base>());
        keyInput->registerFunc(f);

        // TODO
        return nullptr;

        // return ecs->attach<KeyComponent>(obj, keyInput);
    }

}