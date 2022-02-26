#include "listview.h"

#include <iostream>

namespace pg
{
    template<>
    void renderer(MasterRenderer* masterRenderer, SlideBar* slideBar)
    {
        masterRenderer->render(slideBar->slider);
        masterRenderer->render(slideBar->cursor);
    }

    SlideBar::SlideBar(const UiFrame& frame, UiSize* posToUpdate, Orientation orientation) : UiComponent(frame), posUpdate(posToUpdate), orientation(orientation)
    {
        // Default slider
        slider = new TextureComponent(this->width, this->height, "res/object/slider.png");
        slider->setTopAnchor(this->top);
        slider->setLeftAnchor(this->left);

        cursor = new TextureComponent(this->width, this->buttonHeight, "res/object/cursor.png");
        cursor->setTopAnchor(this->top);
        cursor->setLeftAnchor(this->left);
    }

    SlideBar::SlideBar(const UiFrame& frame, const UiFrame& boxToMonitor, const UiSize& maxPos, UiSize* posToUpdate, Orientation orientation) : SlideBar(frame, posToUpdate, orientation)
    {
        this->boxToMonitor = boxToMonitor;

        updateCursorSize(maxPos);
    }

    void SlideBar::mouseInput(Input* inputHandler, double deltaTime...)
    {
        static bool pressed = false;

        if(not inputHandler->isButtonGrabbed(Qt::LeftButton))
            pressed = false;

        if(inputHandler->isButtonPressed(Qt::LeftButton) or pressed) 
        {
            inputHandler->grabMouse(Qt::LeftButton);

            const auto pos = inputHandler->getMousePos();

            auto currentPos = pos.y() - this->pos.y - this->buttonHeight / 2.0f;

            if(currentPos < 0)
                currentPos = 0;

            if(currentPos > height - this->buttonHeight)
                currentPos = height - this->buttonHeight;

            if(posUpdate)
                *posUpdate = currentPos;

            cursor->setTopMargin(currentPos);

            pressed = true;
        }
    }

    void SlideBar::updateCursorSize(const UiSize& maxPos)
    {
        if(maxPos > 0 && this->boxToMonitor.h > 0)
            this->buttonHeight = (this->boxToMonitor.h / maxPos) * height;
        else
            this->buttonHeight = height;

        cursor->setHeight(this->buttonHeight);
    }
    
}