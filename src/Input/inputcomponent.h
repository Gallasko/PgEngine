#pragma once

#include <Qt>

#include "../UI/uisystem.h"
#include "input.h"

#include <cstdarg>

enum class AreaScale : int
{
    FULLSCALE = 1,
    HALFSCALE = 2
};

struct MouseInputComponent
{
	UiPosition *pos;
    UiSize *width, *height; // Input Area
	bool *enable;

    AreaScale scale;

    Base *object;

    void (Base::*onPressed)(Input*, double, ...) = nullptr;

    void (*onPressedLambda)(Input*, double) = nullptr;

    template<typename Func>
    void registerFunc(void (Func::*f)(Input*, double, ...), Func *obj) { onPressed = static_cast<void (Base::*)(Input*, double, ...)>(f); object = static_cast<Base* >(obj); }

    void registerFunc(void (*f)(Input*, double)) { onPressedLambda = f; }

    MouseInputComponent() {}
    //MouseInputComponent(float* x, float* y, float* z, UiSize* width, UiSize* height, bool *enable, AreaScale scale = AreaScale::FULLSCALE) : pos.x(x), pos.y(y), pos.z(z), width(width), height(height), enable(enable), scale(scale) {} 
    MouseInputComponent(UiComponent *component) : pos(&component->pos), width(&component->width), height(&component->height), enable(&component->visible), scale(AreaScale::FULLSCALE) {}
    MouseInputComponent(const MouseInputComponent& component) : pos(component.pos), width(component.width), height(component.height), enable(component.enable), scale(component.scale), object(component.object), onPressed(component.onPressed), onPressedLambda(component.onPressedLambda) {}

    template<typename... Args>
    void call(Input* inputHandler, double deltaTime, Args... args) { if(onPressed != nullptr) (*object.*onPressed)(inputHandler, deltaTime, args...); if(onPressedLambda != nullptr) (*onPressedLambda)(inputHandler, deltaTime); }

    bool inBound(int x, int y) const { return x > this->pos->x / static_cast<int>(this->scale) && x < (this->pos->x + *this->width) / static_cast<int>(this->scale) && y < (this->pos->y + *this->height) / static_cast<int>(this->scale) && y > this->pos->y / static_cast<int>(this->scale); }
    bool inBound(constant::Vector2D vec2) const { return inBound(vec2.x, vec2.y); }

    virtual ~MouseInputComponent() {}
};

template<typename ObjectType>
struct MouseInputBase : public MouseInputComponent
{
    ObjectType *object;

	void (ObjectType::*onPressed)(Input*, double, ...) = nullptr;

    using MouseInputComponent::MouseInputComponent;

    //MouseInputBase() {}
//
    //MouseInputBase(int* x, int* y, int* z, UiSize* width, UiSize* height, bool *enable, AreaScale scale = AreaScale::FULLSCALE) : MouseInputComponent(x, y, z, width, height, enable, scale) {} 
    //MouseInputBase(UiComponent *component) : MouseInputComponent(&component->x, &component->y, &component->z, &component->width, &component->height, &component->visible, AreaScale::FULLSCALE) {}
    //MouseInputBase(const MouseInputBase& component) : MouseInputComponent(component->x, component->y, component->z, component->width, component->height, component->enable, component->scale), onPressed(component.onPressed), object(component.object) { onPressedLambda = component.onPressedLambda; }

    template<typename... Args>
    void call(Input* inputHandler, double deltaTime, Args... args) { if(onPressed != nullptr) {auto obj = static_cast<ObjectType*>(object); auto f = static_cast<void (ObjectType::*)(Input*, double, ...)>(*onPressed); (obj->f)(inputHandler, deltaTime, args...);} if(onPressedLambda != nullptr) (*onPressedLambda)(inputHandler, deltaTime); }

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
    void call(Input* inputHandler, double deltaTime, Args... args) { if(onKey != nullptr) (*object.*onKey)(inputHandler, deltaTime, args...); if(onKeyLambda != nullptr) (*onKeyLambda)(inputHandler, deltaTime); }

    virtual ~KeyboardInputComponent() {}
};

template<typename ObjectType>
struct KeyboardInputBase : public KeyboardInputComponent
{
    ObjectType *object;

	void (ObjectType::*onKey)(Input*, double) = nullptr;

    using KeyboardInputComponent::KeyboardInputComponent;

    //KeyboardInputBase() {}
    //KeyboardInputBase(const KeyboardInputBase& component) : object(component.object), onKey(component.onKey) { onKeyLambda = component.onKeyLambda; }

    template<typename... Args>
    void call(Input* inputHandler, double deltaTime, Args... args) { if(onKey != nullptr) {auto obj = static_cast<ObjectType*>(object); auto f = static_cast<void (ObjectType::*)(Input*, double, ...)>(*onKey); (obj->f)(inputHandler, deltaTime, args...);} if(onKeyLambda != nullptr) (*onKeyLambda)(inputHandler, deltaTime);  }

    ~KeyboardInputBase() {}
};

