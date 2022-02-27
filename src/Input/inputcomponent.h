#pragma once

#include "../UI/uisystem.h"
#include "input.h"

#include <memory>

namespace pg
{
    struct MouseInputComponent
    {
        UiPosition *pos;
        UiSize *width, *height; // Input Area
        bool *enable;

        Base *object;

        void (Base::*onPressed)(Input*, double, ...) = nullptr;

        void (*onPressedLambda)(Input*, double) = nullptr;

        template<typename Func>
        void registerFunc(void (Func::*f)(Input*, double, ...), Func *obj) { onPressed = static_cast<void (Base::*)(Input*, double, ...)>(f); object = static_cast<Base* >(obj); }

        void registerFunc(void (*f)(Input*, double)) { onPressedLambda = f; }

        MouseInputComponent(UiComponent *component) : pos(&component->pos), width(&component->width), height(&component->height), enable(&component->visible) {}
        MouseInputComponent(const MouseInputComponent& component) : pos(component.pos), width(component.width), height(component.height), enable(component.enable), object(component.object), onPressed(component.onPressed), onPressedLambda(component.onPressedLambda) {}

        //TODO check if we can send const Args& all the time or if some specific application need to modify arguments
        template<typename... Args>
        void call(Input* inputHandler, double deltaTime, const Args&... args) { if(onPressed != nullptr) (*object.*onPressed)(inputHandler, deltaTime, args...); if(onPressedLambda != nullptr) (*onPressedLambda)(inputHandler, deltaTime); }

        bool inBound(int x, int y) const { return x > this->pos->x && x < (this->pos->x + *this->width) && y < (this->pos->y + *this->height) && y > this->pos->y; }
        bool inBound(const constant::Vector2D& vec2) const { return inBound(vec2.x, vec2.y); }

        virtual ~MouseInputComponent() {}
    };

    template<typename ObjectType>
    struct MouseInputBase : public MouseInputComponent
    {
        ObjectType *object;

        void (ObjectType::*onPressed)(Input*, double, ...) = nullptr;

        using MouseInputComponent::MouseInputComponent;

        template<typename... Args>
        void call(Input* inputHandler, double deltaTime, const Args&... args) { if(onPressed != nullptr) {auto obj = static_cast<ObjectType*>(object); auto f = static_cast<void (ObjectType::*)(Input*, double, ...)>(*onPressed); (obj->f)(inputHandler, deltaTime, args...);} if(onPressedLambda != nullptr) (*onPressedLambda)(inputHandler, deltaTime); }

        ~MouseInputBase() {}
    };

    struct KeyboardInputComponent
    {
        Base *object;

        void (Base::*onKey)(Input*, double, ...) = nullptr;

        void (*onKeyLambda)(Input*, double) = nullptr;

        KeyboardInputComponent() {}
        KeyboardInputComponent(const KeyboardInputComponent& component) :  object(component.object), onKey(component.onKey), onKeyLambda(component.onKeyLambda) {}

        template<typename Func>
        void registerFunc(void (Func::*f)(Input*, double, ...), Func *obj) { onKey = static_cast<void (Base::*)(Input*, double, ...)>(f); object = static_cast<Base* >(obj); }

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

        /** Logger unique pointer type definition */
        typedef std::unique_ptr<InputSystem> InputPtr;

    public:
        static const InputPtr& system() { static auto system = InputPtr(new InputSystem()); return system; }

        const MouseInputPtr& registerMouseArea(MouseInputPtr component, std::function<void(Input*, double)> callback) { mouseComponents.push_back(component); mouseCallbacks.push_back(callback); return mouseComponents.back(); }
        const KeyInputPtr& registerKeyInput(KeyInputPtr component, std::function<void(Input*, double)> callback) { keyComponents.push_back(component); keyCallbacks.push_back(callback); return keyComponents.back(); }

        void deleteInput(const MouseInputPtr& component) {auto it = findInputPos(component, mouseComponents); if(it != -1) { mouseComponents.erase(mouseComponents.begin() + it); mouseCallbacks.erase(mouseCallbacks.begin() + it); } }
        void deleteInput(const KeyInputPtr& component) {auto it = findInputPos(component, keyComponents); if(it != -1) { keyComponents.erase(keyComponents.begin() + it); keyCallbacks.erase(keyCallbacks.begin() + it); } }

        void updateState(Input* inputHandler, double deltaTime);

    private:
        template <typename Container, typename Value>
        int findInputPos(const Value& value, const Container& container) const
        {
            for (long long unsigned int i = 0; i < container.size(); i++)
                if(value == container.at(i))
                    return i;

            return -1;
        }

        std::vector<MouseInputPtr> mouseComponents; 
        std::vector<KeyInputPtr> keyComponents;

        std::vector<std::function<void(Input*, double)>> mouseCallbacks; 
        std::vector<std::function<void(Input*, double)>> keyCallbacks;
    };

    template<typename ObjectType, typename... Args>
    const MouseInputPtr& makeMouseArea(UiComponent *component, ObjectType *obj, void (ObjectType::*f)(Input*, double, ...), const Args&... args)
    {
        auto& system = InputSystem::system();

        auto mouseArea = MouseInputPtr(new MouseInputBase<ObjectType>(component));
        mouseArea->registerFunc(f, obj);

        auto callback = [=](Input* inputHandler, double deltaTime) { mouseArea->call(inputHandler, deltaTime, args...); };

        return system->registerMouseArea(mouseArea, callback);
    }

    template<typename... Args>
    const MouseInputPtr& makeMouseArea(UiComponent *component, void (*f)(Input*, double), const Args&... args)
    {
        auto& system = InputSystem::system();

        auto mouseArea = MouseInputPtr(new MouseInputBase<Base>(component));
        mouseArea->registerFunc(f);

        auto callback = [=](Input* inputHandler, double deltaTime) { mouseArea->call(inputHandler, deltaTime, args...); };

        return system->registerMouseArea(mouseArea, callback);
    }

    template<typename ObjectType, typename... Args>
    const KeyInputPtr& makeKeyInput(ObjectType *obj, void (ObjectType::*f)(Input*, double, ...), const Args&... args)
    {
        auto& system = InputSystem::system();

        auto keyInput = KeyInputPtr(new KeyboardInputBase<ObjectType>());
        keyInput->registerFunc(f, obj);

        auto callback = [=](Input* inputHandler, double deltaTime) { keyInput->call(inputHandler, deltaTime, args...); };

        return system->registerKeyInput(keyInput, callback);
    }

    template<typename... Args>
    const KeyInputPtr& makeKeyInput(void (*f)(Input*, double), const Args&... args)
    {
        auto& system = InputSystem::system();

        auto keyInput = KeyInputPtr(new KeyboardInputBase<Base>());
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