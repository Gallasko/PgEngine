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
	int *x, *y, *z, *width, *height; // Input Area
	bool focus = false;

	void (*onPressed)(double) = nullptr;
	Qt::MouseButton triggerButton = Qt::LeftButton;

    KeyboardInputComponent() {}
    KeyboardInputComponent(int* x, int* y, int* z, int* width, int* height) : x(x), y(y), z(z), width(width), height(height) {} 
    KeyboardInputComponent(UiComponent *component) : x(&component->x), y(&component->y), z(&component->z), width(&component->width), height(&component->height) {}
    KeyboardInputComponent(const KeyboardInputComponent& component) : x(component.x), y(component.y), z(component.z), width(component.width), height(component.height), focus(component.focus), onPressed(component.onPressed), triggerButton(component.triggerButton) {}

    ~KeyboardInputComponent() {}
};

