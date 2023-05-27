#pragma once

#include "input.h"

#include "../constant.h"

#include "ECS/entitysystem.h"
#include "ECS/callable.h"
#include "UI/uisystem.h"

#include <functional>
#include <memory>

namespace pg
{
    struct MouseLeftClickComponent
    {
        MouseLeftClickComponent(std::shared_ptr<AbstractCallable> callback) : callback(callback) { LOG_THIS_MEMBER("MouseLeftClickSystem"); }
        MouseLeftClickComponent(const MouseLeftClickComponent& rhs) : callback(rhs.callback) { LOG_THIS_MEMBER("MouseLeftClickSystem"); }
        virtual ~MouseLeftClickComponent() { LOG_THIS_MEMBER("MouseLeftClickSystem");}

        std::shared_ptr<AbstractCallable> callback;
    };

    struct MouseRightClickComponent
    {
        MouseRightClickComponent(std::shared_ptr<AbstractCallable> callback) : callback(callback) { LOG_THIS_MEMBER("MouseRightClickSystem"); }
        MouseRightClickComponent(const MouseRightClickComponent& rhs) : callback(rhs.callback) { LOG_THIS_MEMBER("MouseRightClickSystem"); }
        virtual ~MouseRightClickComponent() { LOG_THIS_MEMBER("MouseRightClickSystem");}

        std::shared_ptr<AbstractCallable> callback;
    };

    struct MouseLeaveClickComponent
    {
        MouseLeaveClickComponent(std::shared_ptr<AbstractCallable> callback) : callback(callback) { LOG_THIS_MEMBER("MouseLeaveClickSystem"); }
        MouseLeaveClickComponent(const MouseLeaveClickComponent& rhs) : callback(rhs.callback) { LOG_THIS_MEMBER("MouseLeaveClickSystem"); }
        virtual ~MouseLeaveClickComponent() { LOG_THIS_MEMBER("MouseLeaveClickSystem");}

        std::shared_ptr<AbstractCallable> callback;
    };

    struct OnMouseClick {};

    struct MouseAreaZ
    {
        MouseAreaZ(_unique_id id, CompRef<UiComponent> ui) : id(id), ui(ui) { LOG_THIS_MEMBER("MouseArea"); }

        _unique_id id;
        CompRef<UiComponent> ui;
    };

    struct MouseLeftClickSystem : public System<Own<MouseLeftClickComponent>, NamedSystem, InitSys>
    {
        MouseLeftClickSystem(Input* inputHandler) : inputHandler(inputHandler) { LOG_THIS_MEMBER("MouseLeftClickSystem"); }

        virtual std::string getSystemName() const override { return "Mouse Left Click System"; }

        virtual void init() override
        {
            LOG_THIS_MEMBER("MouseLeftClickSystem");

            auto group = registerGroup<UiComponent, MouseLeftClickComponent>();

            group->addOnGroup([](EntityRef entity) {
                LOG_MILE("MouseLeftClickSystem", "Add entity " << entity->id << " to ui - mouse left click group !");

                auto sys = entity->world()->getSystem<MouseLeftClickSystem>();

                const auto& ui = entity->get<UiComponent>();
                
                sys->mouseAreaHolder.emplace(entity->id, ui);
            });
        }

        virtual void execute() override
        {
            LOG_THIS_MEMBER("MouseLeftClickSystem");

            int highestZ = INT_MIN;
            const auto& mousePos = inputHandler->getMousePos();

            static bool pressed = false;

            if(inputHandler->isButtonPressed(Qt::LeftButton))
            {
                if(not pressed)
                    ecsRef->sendEvent(OnMouseClick{});

                pressed = true;
            }

            if(not inputHandler->isButtonPressed(Qt::LeftButton))
            {
                if(pressed)
                {
                    for(const auto& mouseArea : mouseAreaHolder)
                    {
                        UiComponent *ui = mouseArea.ui;

                        if(ui->pos.z < highestZ)
                            break;

                        if(ui->inBound(mousePos.x(), mousePos.y()))
                        {
                            highestZ = static_cast<UiSize>(ui->pos.z);

                            auto comp = getComponent(mouseArea.id);

                            comp->callback->call(world());
                        }
                    }
                }

                pressed = false;
            }
            
        }

        Input *inputHandler;
        std::set<MouseAreaZ, std::greater<>> mouseAreaHolder;
    };

    struct MouseRightClickSystem : public System<Own<MouseRightClickComponent>, NamedSystem, InitSys>
    {
        MouseRightClickSystem(Input* inputHandler) : inputHandler(inputHandler) { LOG_THIS_MEMBER("MouseRightClickSystem"); }

        virtual std::string getSystemName() const override { return "Mouse Right Click System"; }

        virtual void init() override
        {
            LOG_THIS_MEMBER("MouseRightClickSystem");

            auto group = registerGroup<UiComponent, MouseRightClickComponent>();

            group->addOnGroup([](EntityRef entity) {
                LOG_MILE("MouseRightClickSystem", "Add entity " << entity->id << " to ui - mouse right click group !");

                auto sys = entity->world()->getSystem<MouseRightClickSystem>();

                const auto& ui = entity->get<UiComponent>();
                
                sys->mouseAreaHolder.emplace(entity->id, ui);
            });
        }

        virtual void execute() override
        {
            LOG_THIS_MEMBER("MouseRightClickSystem");

            int highestZ = INT_MIN;
            const auto& mousePos = inputHandler->getMousePos();

            static bool pressed = false;

            if(inputHandler->isButtonPressed(Qt::RightButton))
            {
                if(not pressed)
                    ecsRef->sendEvent(OnMouseClick{});

                pressed = true;
            }

            if(not inputHandler->isButtonPressed(Qt::RightButton))
            {
                if(pressed)
                {
                    for(const auto& mouseArea : mouseAreaHolder)
                    {
                        UiComponent *ui = mouseArea.ui;

                        if(ui->pos.z < highestZ)
                            break;

                        if(ui->inBound(mousePos.x(), mousePos.y()))
                        {
                            highestZ = static_cast<UiSize>(ui->pos.z);

                            auto comp = getComponent(mouseArea.id);

                            comp->callback->call(world());
                        }
                    }
                }

                pressed = false;
            }
            
        }

        Input *inputHandler;
        std::set<MouseAreaZ, std::greater<>> mouseAreaHolder;
    };

    struct MouseLeaveClickSystem : public System<Listener<OnMouseClick>, Own<MouseLeaveClickComponent>, NamedSystem, InitSys, StoragePolicy>
    {
        MouseLeaveClickSystem(Input* inputHandler) : inputHandler(inputHandler) { LOG_THIS_MEMBER("MouseLeaveClickSystem"); }

        virtual std::string getSystemName() const override { return "Mouse Leave Click System"; }

        virtual void init() override
        {
            LOG_THIS_MEMBER("MouseLeaveClickSystem");

            auto group = registerGroup<UiComponent, MouseLeaveClickComponent>();

            group->addOnGroup([](EntityRef entity) {
                LOG_MILE("MouseLeaveClickSystem", "Add entity " << entity->id << " to ui - mouse leave click group !");

                auto sys = entity->world()->getSystem<MouseLeaveClickSystem>();

                const auto& ui = entity->get<UiComponent>();
                
                sys->mouseAreaHolder.emplace(entity->id, ui);
            });
        }

        virtual void onEvent(const OnMouseClick&) override
        {
            LOG_THIS_MEMBER("MouseLeaveClickSystem");

            const auto& mousePos = inputHandler->getMousePos();

            for(const auto& mouseArea : mouseAreaHolder)
            {
                UiComponent *ui = mouseArea.ui;

                if(not ui->inBound(mousePos.x(), mousePos.y()))
                {
                    auto comp = getComponent(mouseArea.id);

                    comp->callback->call(world());
                }
            }
        }

        Input *inputHandler;
        std::set<MouseAreaZ, std::less<>> mouseAreaHolder;
    };

    bool operator<(const MouseAreaZ& lhs, const MouseAreaZ& rhs);
    bool operator>(const MouseAreaZ& lhs, const MouseAreaZ& rhs);

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

        template <typename Func>
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
        template <typename... Args>
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

    template <typename ObjectType>
    struct MouseInputBase : public MouseInputComponent
    {
        ObjectType *object;

        void (ObjectType::*onPressed)(Input*, double, ...) = nullptr;
        void (ObjectType::*onLeave)(Input*, double) = nullptr;

        using MouseInputComponent::MouseInputComponent;

        template <typename... Args>
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

        template <typename Func>
        void registerFunc(void (Func::*f)(Input*, double, ...), Func *obj)
        { 
            struct Delegate : public Func, public Base {};

            onKey = static_cast<void (Base::*)(Input*, double, ...)>( static_cast<void (Delegate::*)(Input*, double, ...)>(f) );
            object = static_cast<Delegate* >(obj);
        }

        void registerFunc(void (*f)(Input*, double)) { onKeyLambda = f; }

        template <typename... Args>
        void call(Input* inputHandler, double deltaTime, const Args&... args) { if(onKey != nullptr) (*object.*onKey)(inputHandler, deltaTime, args...); if(onKeyLambda != nullptr) (*onKeyLambda)(inputHandler, deltaTime); }

        virtual ~KeyboardInputComponent() {}
    };

    template <typename ObjectType>
    struct KeyboardInputBase : public KeyboardInputComponent
    {
        ObjectType *object;

        void (ObjectType::*onKey)(Input*, double, ...) = nullptr;

        using KeyboardInputComponent::KeyboardInputComponent;

        template <typename... Args>
        void call(Input* inputHandler, double deltaTime, const Args&... args) { if(onKey != nullptr) {auto obj = static_cast<ObjectType*>(object); auto f = static_cast<void (ObjectType::*)(Input*, double, ...)>(*onKey); (obj->f)(inputHandler, deltaTime, args...);} if(onKeyLambda != nullptr) (*onKeyLambda)(inputHandler, deltaTime);  }

        ~KeyboardInputBase() {}
    };

    typedef std::shared_ptr<MouseInputComponent> MouseInputPtr;
    typedef std::shared_ptr<KeyboardInputComponent> KeyInputPtr;

    // Helper Struct
    struct MouseComponent
    {
        MouseComponent(MouseInputPtr comp) : component(comp) {} 

        MouseInputPtr component;
    };

    // Helper Struct
    struct KeyComponent
    {
        KeyComponent(KeyInputPtr comp) : component(comp) {} 

        KeyInputPtr component;
    };

    class InputSystem : public System<Own<MouseComponent>, Own<KeyComponent>, NamedSystem>
    {
        /** InputSystem unique pointer type definition */
        typedef std::unique_ptr<InputSystem> InputPtr;

    public:
        InputSystem() : System<Own<MouseComponent>, Own<KeyComponent>, NamedSystem>()
        {
            setPolicy(ExecutionPolicy::Storage);
        }

        virtual std::string getSystemName() const override { return "Input System"; }

        void execute() override
        {
            for(const auto& mouse : view<MouseComponent>())
            {
                // mouse->component->call(inputHandler, deltaTime);
            }
        }
    
        template <typename ObjectType, typename... Args>
        friend Entity* makeKeyInput(EntitySystem *ecs, ObjectType *obj, void (ObjectType::*f)(Input*, double, ...), const Args&... args);

        template <typename... Args>
        friend Entity* makeKeyInput(EntitySystem *ecs, void (*f)(Input*, double), const Args&... args);

    private:
        void reorderMouse();
    };

    template <typename ObjectType, typename... Args>
    Entity* makeMouseArea(EntitySystem *ecs, UiComponent *component, ObjectType *obj, void (ObjectType::*mouseInput)(Input*, double, ...), void (ObjectType::*mouseLeave)(Input*, double));

    template <typename ObjectType, typename... Args>
    Entity* makeMouseArea(EntitySystem *ecs, UiComponent *component, ObjectType *obj, void (ObjectType::*mouseInput)(Input*, double, ...), std::nullptr_t);

    template <typename ObjectType, typename... Args>
    Entity* makeMouseArea(EntitySystem *ecs, UiComponent *component, ObjectType *obj, void (ObjectType::*mouseInput)(Input*, double, ...));

    Entity* makeMouseArea(EntitySystem *ecs, UiComponent *component, void (*mouseInput)(Input*, double), void (*mouseLeave)(Input*, double));
    Entity* makeMouseArea(EntitySystem *ecs, UiComponent *component, void (*mouseInput)(Input*, double), std::nullptr_t);
    Entity* makeMouseArea(EntitySystem *ecs, UiComponent *component, void (*mouseInput)(Input*, double));
    Entity* makeMouseArea(EntitySystem *ecs, UiComponent *component, const std::function<void (Input*, double)>& mouseInput, const std::function<void (Input*, double)>& mouseLeave);
    Entity* makeMouseArea(EntitySystem *ecs, UiComponent *component, const std::function<void (Input*, double)>& mouseInput, std::nullptr_t);
    Entity* makeMouseArea(EntitySystem *ecs, UiComponent *component, const std::function<void (Input*, double)>& mouseInput);

    template <typename ObjectType, typename... Args>
    Entity* makeMouseArea(EntitySystem *ecs, UiComponent *component, ObjectType *obj, void (ObjectType::*mouseInput)(Input*, double, ...), void (ObjectType::*mouseLeave)(Input*, double))
    {
        if(not ecs)
            return nullptr;

        MouseInputPtr mouseArea = std::make_shared<MouseInputBase<ObjectType>>(component);
        mouseArea->registerFunc(mouseInput, obj, mouseLeave);

        auto ent = ecs->createEntity();

        ecs->attach<MouseComponent>(ent, mouseArea);

        return ent;
    }

    template <typename ObjectType, typename... Args>
    Entity* makeMouseArea(EntitySystem *ecs, UiComponent *component, ObjectType *obj, void (ObjectType::*mouseInput)(Input*, double, ...), std::nullptr_t)
    {
        if(not ecs)
            return nullptr;

        MouseInputPtr mouseArea = std::make_shared<MouseInputBase<ObjectType>>(component);
        mouseArea->registerFunc(mouseInput, obj, static_cast<void (ObjectType::*)(pg::Input*, double)>(nullptr));

        auto ent = ecs->createEntity();

        ecs->attach<MouseComponent>(ent, mouseArea);

        return ent;
    }

    template <typename ObjectType>
    Entity* makeMouseArea(EntitySystem *ecs, UiComponent *component, ObjectType *obj, void (ObjectType::*mouseInput)(Input*, double, ...))
    {
        return makeMouseArea(ecs, component, obj, mouseInput, nullptr);
    }

    template <typename ObjectType, typename... Args>
    Entity* makeKeyInput(EntitySystem *ecs, ObjectType *obj, void (ObjectType::*f)(Input*, double, ...), const Args&... args)
    {
        if(not ecs)
            return nullptr;

        KeyInputPtr keyInput = std::make_shared<KeyboardInputBase<ObjectType>>();
        keyInput->registerFunc(f, obj);

        auto ent = ecs->createEntity();

        ecs->attach<KeyComponent>(ent, keyInput);

        return ent;
    }

    template <typename... Args>
    Entity* makeKeyInput(EntitySystem *ecs, void (*f)(Input*, double), const Args&... args)
    {
        if(not ecs)
            return nullptr;

        KeyInputPtr keyInput = std::make_shared<KeyboardInputBase<KeyboardInputComponent::Base>>();
        keyInput->registerFunc(f);

        auto ent = ecs->createEntity();

        ecs->attach<KeyComponent>(ent, keyInput);

        return ent;
    }

}