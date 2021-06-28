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

struct KeyboardInputComponent
{
    bool focus = false;

	void (*onKey)(Input*, double) = [](Input* inputHandler, double deltaTime) {};

    KeyboardInputComponent() {}
    KeyboardInputComponent(const KeyboardInputComponent& component) : focus(component.focus), onKey(component.onKey) {}

    //void call(Input* inputHandler, double deltaTime) { (*object.*onKey)(inputHandler, deltaTime); }

    ~KeyboardInputComponent() {}
};

