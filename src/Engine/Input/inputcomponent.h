#pragma once

#include "input.h"

#include "../constant.h"

#include <functional>
#include <memory>

namespace pg
{
    // Definition forwarding
    class UiPosition;
    class UiSize;
    class UiComponent;

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
        }

        // TODO see if leave function need more args or not
        void leave(Input* inputHandler, double deltaTime)
        {
            if(onLeave != nullptr)
                (*object.*onLeave)(inputHandler, deltaTime);

            if(onLeaveLambda != nullptr) 
                (*onLeaveLambda)(inputHandler, deltaTime);
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

    class MouseInput;

    struct InputIndice
    {
        int index = 0;
        InputIndice *next = nullptr;
    };

    class InputSystem 
    {
        friend class MouseInput;

        // Typedefs

        /** InputSystem unique pointer type definition */
        typedef std::unique_ptr<InputSystem> InputPtr;

    public:
        // Helper Struct
        struct MouseComponent
        {
            MouseComponent(const MouseInputPtr& component, InputIndice* indice, const std::function<void(Input*, double)>& inputCallback, const std::function<void(Input*, double)>& leaveCallback = nullptr) : component(component), inputCallback(inputCallback), leaveCallback(leaveCallback), indice(indice) {}
            
            bool operator==(const MouseComponent& rhs) const { return component == rhs.component; }

            MouseInputPtr component;
            std::function<void(Input*, double)> inputCallback;
            std::function<void(Input*, double)> leaveCallback;
            InputIndice* indice = nullptr;
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
        MouseInput makeMouseArea(UiComponent *component, ObjectType *obj, void (ObjectType::*mouseInput)(Input*, double, ...), void (ObjectType::*mouseLeave)(Input*, double), const Args&... args);

        template<typename ObjectType, typename... Args>
        MouseInput makeMouseArea(UiComponent *component, ObjectType *obj, void (ObjectType::*mouseInput)(Input*, double, ...), std::nullptr_t, const Args&... args);

        template<typename ObjectType, typename... Args>
        MouseInput makeMouseArea(UiComponent *component, ObjectType *obj, void (ObjectType::*mouseInput)(Input*, double, ...));
    
        friend MouseInput makeMouseArea(UiComponent *component, void (*mouseInput)(Input*, double), void (*mouseLeave)(Input*, double));
        friend MouseInput makeMouseArea(UiComponent *component, void (*mouseInput)(Input*, double), std::nullptr_t);
        friend MouseInput makeMouseArea(UiComponent *component, void (*mouseInput)(Input*, double));
        friend MouseInput makeMouseArea(UiComponent *component, const std::function<void(pg::Input*, double)>& mouseInput);
        friend MouseInput makeMouseArea(UiComponent *component, const std::function<void(pg::Input*, double)>& mouseInput, const std::function<void(pg::Input*, double)>& mouseLeave);
    
        friend MouseInput makeMouseArea(UiComponent *component, const InputSystem::MouseComponent& mouseArea);

        template<typename ObjectType, typename... Args>
        friend const InputSystem::KeyComponent& makeKeyInput(ObjectType *obj, void (ObjectType::*f)(Input*, double, ...), const Args&... args);

        template<typename... Args>
        friend const InputSystem::KeyComponent& makeKeyInput(void (*f)(Input*, double), const Args&... args);

        MouseInput registerMouseArea(MouseInputPtr component, const std::function<void(Input*, double)>& inputCallback, const std::function<void(Input*, double)>& leaveCallback = nullptr);
        const InputSystem::KeyComponent& registerKeyInput(KeyInputPtr component, const std::function<void(Input*, double)>& callback);

    private:
        InputIndice* findLastIndice() const
        {
            InputIndice* indice = firstIndice;

            if(indice == nullptr)
                return nullptr;

            while(indice->next != nullptr)
                indice = indice->next;

            return indice;
        };

        void reorderMouse() {};
        void deleteMouseInput(int index)
        {

        };

        // Storing unique ptr of the component to avoid invaliding the ref to the component
        std::vector<InputSystem::MouseComponent> mouseComponents;
        std::vector<InputSystem::KeyComponent> keyComponents;

        InputIndice *firstIndice = nullptr;

        std::vector<int> mouseDeleteList;
    };

    class MouseInput 
    {
    friend class InputSystem;
    public:
        MouseInputPtr operator->() const { return InputSystem::system()->mouseComponents[indice->index].component; }

        void changeZ(const UiSize& zOrder) const;

        void deleteInput() const
        {
            InputSystem::system()->deleteMouseInput(indice->index);
        }

    private:
        InputIndice *indice;
    };

    MouseInput makeMouseArea(UiComponent *component, void (*mouseInput)(Input*, double), void (*mouseLeave)(Input*, double));
    MouseInput makeMouseArea(UiComponent *component, void (*mouseInput)(Input*, double), std::nullptr_t);
    MouseInput makeMouseArea(UiComponent *component, void (*mouseInput)(Input*, double));
    MouseInput makeMouseArea(UiComponent *component, const std::function<void(pg::Input*, double)>& mouseInput);
    MouseInput makeMouseArea(UiComponent *component, const std::function<void(pg::Input*, double)>& mouseInput, const std::function<void(pg::Input*, double)>& mouseLeave);
    MouseInput makeMouseArea(UiComponent *component, const InputSystem::MouseComponent& mouseArea);

    template<typename ObjectType, typename... Args>
    MouseInput makeMouseArea(UiComponent *component, ObjectType *obj, void (ObjectType::*mouseInput)(Input*, double, ...), void (ObjectType::*mouseLeave)(Input*, double), const Args&... args)
    {
        auto& system = InputSystem::system();

        auto mouseArea = MouseInputPtr(new MouseInputBase<ObjectType>(component));
        mouseArea->registerFunc(mouseInput, obj, mouseLeave);

        auto inputCallback = [=](Input* inputHandler, double deltaTime) { mouseArea->call(inputHandler, deltaTime, args...); };

        std::function<void(Input*, double)> leaveCallback = [=](Input* inputHandler, double deltaTime) { mouseArea->leave(inputHandler, deltaTime); };

        return system->registerMouseArea(mouseArea, inputCallback, leaveCallback);
    }

    template<typename ObjectType, typename... Args>
    MouseInput makeMouseArea(UiComponent *component, ObjectType *obj, void (ObjectType::*mouseInput)(Input*, double, ...), std::nullptr_t, const Args&... args)
    {
        auto& system = InputSystem::system();

        auto mouseArea = MouseInputPtr(new MouseInputBase<ObjectType>(component));
        mouseArea->registerFunc(mouseInput, obj, static_cast<void (ObjectType::*)(pg::Input*, double)>(nullptr));

        auto inputCallback = [=](Input* inputHandler, double deltaTime) { mouseArea->call(inputHandler, deltaTime, args...); };

        return system->registerMouseArea(mouseArea, inputCallback);
    }

    template<typename ObjectType>
    MouseInput makeMouseArea(UiComponent *component, ObjectType *obj, void (ObjectType::*mouseInput)(Input*, double, ...))
    {
        auto& system = InputSystem::system();

        auto mouseArea = MouseInputPtr(new MouseInputBase<ObjectType>(component));
        mouseArea->registerFunc(mouseInput, obj, static_cast<void (ObjectType::*)(pg::Input*, double)>(nullptr));

        auto inputCallback = [=](Input* inputHandler, double deltaTime) { mouseArea->call(inputHandler, deltaTime); };

        return system->registerMouseArea(mouseArea, inputCallback);
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

    // TODO 
    /*
    template<typename Input>
    void deleteInput(const Input& input)
    {
        auto& system = InputSystem::system();

        system->deleteInput(input);
    }
    */

    void deleteInput(const MouseInput& input);
}