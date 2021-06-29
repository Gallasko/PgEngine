#pragma once

#include <Qt>

#include "../UI/uisystem.h"
#include "input.h"

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

	void (*onPressed)(Input*, double) = [](Input* inputHandler, double deltaTime) {};

    MouseInputComponent() {}
    MouseInputComponent(int* x, int* y, int* z, int* width, int* height, bool *enable, AreaScale scale = AreaScale::FULLSCALE) : x(x), y(y), z(z), width(width), height(height), enable(enable), scale(scale) {} 
    MouseInputComponent(UiComponent *component) : x(&component->x), y(&component->y), z(&component->z), width(&component->width), height(&component->height), enable(&component->visible), scale(AreaScale::HALFSCALE) {}
    MouseInputComponent(const MouseInputComponent& component) : x(component.x), y(component.y), z(component.z), width(component.width), height(component.height), enable(component.enable), focus(component.focus), scale(component.scale), onPressed(component.onPressed) {}

    ~MouseInputComponent() {}
};

struct Obj {};

struct KeyboardInputComponent
{
    bool focus = false;

    Obj *object;

	void (Obj::*onKey)(Input*, double) = nullptr; //= [](Input* inputHandler, double deltaTime) {};

    KeyboardInputComponent() {}
    KeyboardInputComponent(const KeyboardInputComponent& component) : focus(component.focus), onKey(component.onKey) {}

    template<typename Func>
    void registerFunc(void (Func::*f)(Input*, double), Func *obj) { onKey = static_cast<void (Obj::*)(Input*, double)>(f); object = static_cast<Obj* >(obj); }

    void call(Input* inputHandler, double deltaTime) { (*object.*onKey)(inputHandler, deltaTime); }

    ~KeyboardInputComponent() {}
};

template<typename ObjectType>
struct KeyboardInputBase : public KeyboardInputComponent
{
    bool focus = false;

    ObjectType *object;

	void (ObjectType::*onKey)(Input*, double) = nullptr; //= [](Input* inputHandler, double deltaTime) {};

    KeyboardInputBase() {}
    KeyboardInputBase(const KeyboardInputBase& component) : focus(component.focus), onKey(component.onKey) {}

    void call(Input* inputHandler, double deltaTime) { auto obj = static_cast<ObjectType*>(object); auto f = static_cast<void (ObjectType::*)(Input*, double)>(*onKey); (obj->f)(inputHandler, deltaTime); }

    ~KeyboardInputBase() {}
};

