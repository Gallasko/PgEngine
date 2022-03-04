#pragma once

#include "../UI/uisystem.h"
#include "input.h"

#include <memory>

namespace pg
{
    /**
     * @struct MouseInputComponent
     * @brief Mouse Input Component
     * 
     * A structure that holds all the data needed to manage a mouse input component.
     * This structure is created throught the Mouse Input Base to get the correct templated function !
     * 
     * @see MouseInputBase
     */
    struct MouseInputComponent
    {
        /** Base struct to convert any class member and store them as generic function pointer */
        struct Base {};

        /** Frame where the mouse input happened */
        const UiFrame *frame;
        /** Pointer to the visible parameters to see if the area is visible and accessible */
        bool *enable;

        /** Pointer to the object where the functions needs to be called */
        Base *object;

        /** Pointer to the class member function to call when the mouse is in the area */
        void (Base::*onPressed)(Input*, double, ...) = nullptr;
        /** Pointer to the class member function to call when the mouse is not in the area */
        void (Base::*onLeave)(Input*, double) = nullptr;

        /** Pointer to the lambda function to call when the mouse is in the area */
        void (*onPressedLambda)(Input*, double) = nullptr;
        /** Pointer to the lambda function to call when the mouse is not in the area */
        void (*onLeaveLambda)(Input*, double) = nullptr;

        /**
         * @fn registerFunc
         * @brief Function to register an object and its mouseInput and mouseLeave callback
         * 
         * @tparam Object The type of the object that we be called from the callback function 
         * @param mouseInput a callback function that get called when the mouse is in the area defined
         * @param obj a pointer to the object called
         * @param mouseLeave 'Optional' a callback function that get called when the mouse is not in the area  
         */
        template<typename Object>
        void registerFunc(void (Object::*mouseInput)(Input*, double, ...), Object *obj, void (Object::*mouseLeave)(Input*, double) = nullptr)
        {
            // A Helper struct to convert any Object to a Base class to archive type erasure
            struct Delegate : public Object, public Base {};

            // Cast the function to the delegate then to base to erase the input type
            onPressed = static_cast<void (Base::*)(Input*, double, ...)>( static_cast<void (Delegate::*)(Input*, double, ...)>(mouseInput) );
            onLeave = static_cast<void (Base::*)(Input*, double)>( static_cast<void (Delegate::*)(Input*, double)>(mouseLeave) );

            // Cast the object to delegate then it get implicitly cast to Base
            object = static_cast<Delegate* >(obj);
        }

        /**
         * @fn registerFunc
         * @brief Overload function of registerFunc for lambdas
         * 
         * @param mouseInput a callback function that get called when the mouse is in the area defined
         * @param mouseLeave 'Optional' a callback function that get called when the mouse is not in the area
         */
        void registerFunc(void (*mouseEnter)(Input*, double), void (*mouseLeave)(Input*, double) = nullptr)
        {
            onPressedLambda = mouseEnter;
            onLeaveLambda = mouseLeave;
        }

        /**
         * @brief Construct a new Mouse Input Component object
         * 
         * @param component a pointer to an UiComponent to get the area where mouse input are handled
         */
        MouseInputComponent(UiComponent *component) : frame(&component->frame), enable(&component->visible) {}

        /**
         * @brief Copy and construct a new Mouse Input Component object
         * 
         * @param component the Mouse Input Component to copy
         */
        MouseInputComponent(const MouseInputComponent& component) : frame(component.frame), enable(component.enable), object(component.object), onPressed(component.onPressed), onLeave(component.onLeave), onPressedLambda(component.onPressedLambda), onLeaveLambda(component.onLeaveLambda) {}

        /**
         * @fn call
         * @brief Call the mouseInput and mouseInputLambda functions registered
         * TODO check if we can send const Args& all the time or if some specific application need to modify arguments
         * 
         * @tparam Args Type of the arguments of the mouse input function
         * @param inputHandler a pointer to the input handler
         * @param deltaTime time passed since the last call
         * @param args arguments of the mouse input function
         */
        template<typename... Args>
        void call(Input* inputHandler, double deltaTime, const Args&... args)
        {
            if(onPressed != nullptr)
                (*object.*onPressed)(inputHandler, deltaTime, args...);
            
            if(onPressedLambda != nullptr) 
                (*onPressedLambda)(inputHandler, deltaTime);
        }

        /**
         * @fn leave
         * @brief Call the mouseLeave and mouseLeaveLambda functions registered
         * TODO check if leave function need more args or not
         * 
         * @param inputHandler a pointer to the input handler
         * @param deltaTime time passed since the last call
         */
        void leave(Input* inputHandler, double deltaTime)
        {
            if(onLeave != nullptr)
                (*object.*onLeave)(inputHandler, deltaTime);

            if(onLeaveLambda != nullptr) 
                (*onLeaveLambda)(inputHandler, deltaTime);
        }

        /**
         * @fn inBound
         * @brief Calculate if a point is in the Mouse Area
         * 
         * @param x x coordinate of the point
         * @param y y coordinate of the point
         * @return true if the point is inside the area, false otherwise
         */
        bool inBound(int x, int y) const { return x > this->frame->pos.x && x < (this->frame->pos.x + this->frame->w) && y < (this->frame->pos.y + this->frame->h) && y > this->frame->pos.y; }
        
        /**
         * @fn inBound
         * @brief Overload of the inBound(int x, int y) function, call exctract x and y and pass it to the function
         * 
         * @param vec2 a vector of the coordinate of the point
         * @return true if the point is inside the area, false otherwise
         */
        bool inBound(const constant::Vector2D& vec2) const { return inBound(vec2.x, vec2.y); }

        /** Virtual destructor of MouseInputComponent */
        virtual ~MouseInputComponent() {}
    };

    /**
     * @struct MouseInputBase
     * @brief Mouse Input Base
     * 
     * @tparam ObjectType the type of object which will call the function of the mouse area
     * 
     * Base of the Mouse Input Component, it initialize a mouse input component for a given object.
     * It permits the use of members of the object to be call inside the mouse area.
     * 
     * @see MouseInputComponent
     */
    template<typename ObjectType>
    struct MouseInputBase : public MouseInputComponent
    {
        /** The object which will call the function */
        ObjectType *object;

        /** Pointer to the member to call on mouse input inside the mouse area */
        void (ObjectType::*onPressed)(Input*, double, ...) = nullptr;
        
        /** Pointer to the member to call on mouse input outside the mouse area */
        void (ObjectType::*onLeave)(Input*, double) = nullptr;

        // Import all base constructor from MouseInputComponent
        using MouseInputComponent::MouseInputComponent;

        /**
         * @fn call
         * @brief Call the mouseInput and mouseInputLambda functions registered
         * This hide the Base call function and cast all the function that had they type erased in the correct Type
         * 
         * @tparam Args Type of the arguments of the mouse input function
         * @param inputHandler a pointer to the input handler
         * @param deltaTime time passed since the last call
         * @param args arguments of the mouse input function
         */
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

        /**
         * @fn leave
         * @brief Call the mouseLeave and mouseLeaveLambda functions registered
         * This hide the Base call function and cast all the function that had they type erased in the correct Type
         * 
         * @param inputHandler a pointer to the input handler
         * @param deltaTime time passed since the last call
         */
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

        /** Virtual destructor of MouseInputBase */
        virtual ~MouseInputBase() {}
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