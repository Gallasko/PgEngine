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
	int *x, *y, *z, *width, *height; // Input Area
	bool *enable;
    bool focus = false;

    AreaScale scale;

    Base *object;

    void (Base::*onPressed)(Input*, double, ...) = nullptr;

    void (*onPressedLambda)(Input*, double) = nullptr;

    template<typename Func>
    void registerFunc(void (Func::*f)(Input*, double, ...), Func *obj) { onPressed = static_cast<void (Base::*)(Input*, double, ...)>(f); object = static_cast<Base* >(obj); }

    void registerFunc(void (*f)(Input*, double)) { onPressedLambda = f; }

    MouseInputComponent() {}
    MouseInputComponent(int* x, int* y, int* z, int* width, int* height, bool *enable, AreaScale scale = AreaScale::FULLSCALE) : x(x), y(y), z(z), width(width), height(height), enable(enable), scale(scale) {} 
    MouseInputComponent(UiComponent *component) : x(&component->x), y(&component->y), z(&component->z), width(&component->width), height(&component->height), enable(&component->visible), scale(AreaScale::HALFSCALE) {}
    MouseInputComponent(TextureComponent *component) : x(&component->x), y(&component->y), z(&component->z), width(&component->width), height(&component->height), enable(&component->visible), scale(AreaScale::FULLSCALE) {}
    MouseInputComponent(const MouseInputComponent& component) : x(component.x), y(component.y), z(component.z), width(component.width), height(component.height), enable(component.enable), focus(component.focus), scale(component.scale), onPressed(component.onPressed), onPressedLambda(component.onPressedLambda), object(component.object) {}

    template<typename... Args>
    void call(Input* inputHandler, double deltaTime, Args... args) { if(onPressed != nullptr) (*object.*onPressed)(inputHandler, deltaTime, args...); if(onPressedLambda != nullptr) (*onPressedLambda)(inputHandler, deltaTime); }

    virtual ~MouseInputComponent() {}
};

template<typename ObjectType>
struct MouseInputBase : public MouseInputComponent
{
    bool focus = false;

    AreaScale scale;

    ObjectType *object;

	void (ObjectType::*onPressed)(Input*, double, ...) = nullptr;

    MouseInputBase() {}

    MouseInputBase(int* x, int* y, int* z, int* width, int* height, bool *enable, AreaScale scale = AreaScale::FULLSCALE) : MouseInputComponent(x, y, z, width, height, enable, scale) {} 
    MouseInputBase(UiComponent *component) : MouseInputComponent(&component->x, &component->y, &component->z, &component->width, &component->height, &component->visible, AreaScale::HALFSCALE) {}
    MouseInputBase(TextureComponent *component) : MouseInputComponent(&component->x, &component->y, &component->z, &component->width, &component->height, &component->visible, AreaScale::FULLSCALE) {}
    MouseInputBase(const MouseInputBase& component) : MouseInputComponent(component->x, component->y, component->z, component->width, component->height, component->enable, component->scale), focus(component.focus), onPressed(component.onPressed), object(component.object) { onPressedLambda = component.onPressedLambda; }

    template<typename... Args>
    void call(Input* inputHandler, double deltaTime, Args... args) { if(onPressed != nullptr) {auto obj = static_cast<ObjectType*>(object); auto f = static_cast<void (ObjectType::*)(Input*, double, ...)>(*onPressed); (obj->f)(inputHandler, deltaTime, args...);} if(onPressedLambda != nullptr) (*onPressedLambda)(inputHandler, deltaTime); }

    ~MouseInputBase() {}
};

struct KeyboardInputComponent
{
    bool focus = false;

    Base *object;

	void (Base::*onKey)(Input*, double, ...) = nullptr;

	void (*onKeyLambda)(Input*, double) = nullptr;

    KeyboardInputComponent() {}
    KeyboardInputComponent(const KeyboardInputComponent& component) : focus(component.focus), onKey(component.onKey), onKeyLambda(component.onKeyLambda), object(component.object) {}

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
    bool focus = false;

    ObjectType *object;

	void (ObjectType::*onKey)(Input*, double) = nullptr;

    KeyboardInputBase() {}
    KeyboardInputBase(const KeyboardInputBase& component) : focus(component.focus), onKey(component.onKey), object(component.object) { onKeyLambda = component.onKeyLambda; }

    template<typename... Args>
    void call(Input* inputHandler, double deltaTime, Args... args) { if(onKey != nullptr) {auto obj = static_cast<ObjectType*>(object); auto f = static_cast<void (ObjectType::*)(Input*, double, ...)>(*onKey); (obj->f)(inputHandler, deltaTime, args...);} if(onKeyLambda != nullptr) (*onKeyLambda)(inputHandler, deltaTime);  }

    ~KeyboardInputBase() {}
};

