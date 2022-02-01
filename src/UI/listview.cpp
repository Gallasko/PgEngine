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

    SlideBar::SlideBar(const UiFrame& frame, UiSize* posToUpdate, Orientation orientation) : UiComponent(frame), orientation(orientation), yMin(0.0f, 0.0f, nullptr), yMax(0.0f, 1.0f, &this->pos.y)
    {
        posUpdate = posToUpdate;

        // Default slider
        slider = new TextureComponent(this->width, this->height, "res/object/slider.png");
        slider->setTopAnchor(this->top);
        slider->setLeftAnchor(this->left);

        cursor = new TextureComponent(16, 50, "res/object/cursor.png");
        cursor->setTopAnchor(this->top);
        cursor->setLeftAnchor(this->left);
        cursor->setLeftMargin(2);
    }

    void SlideBar::mouseInput(Input* inputHandler, double deltaTime...)
    {
        if(inputHandler->isButtonPressed(Qt::LeftButton)) 
        {
            const auto pos = inputHandler->getMousePos();

            cursor->setTopMargin(pos.y() - this->pos.y);
        } 
    }
    
}