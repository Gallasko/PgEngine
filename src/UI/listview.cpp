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

    SlideBar::SlideBar(const UiFrame& frame, UiSize* posToUpdate) : yMin(0.0f, 0.0f, nullptr), yMax(0.0f, 1.0f, &this->pos.y)
    {
        this->pos.x = &frame.x;
        this->pos.y = &frame.y;

        this->width = &frame.w;
        this->height = &frame.h;

        posUpdate = posToUpdate;

        // Default slider
        slider = new TextureComponent(this->width, this->height, "/res/object/slider.png");
        slider->setTopAnchor(this->topAnchor);
        slider->setLeftAnchor(this->leftAnchor);

        cursor = new TextureComponent(16, 24, "/res/object/cursor.png");
        cursor->setTopAnchor(this->topAnchor);
        cursor->setLeftAnchor(this->leftAnchor);
        cursor->setLeftMargin(2);
    }

    void SlideBar::mouseInput(Input* inputHandler, double deltaTime...)
    {
        static bool pressed = false;

        if(inputHandler->isButtonPressed(Qt::LeftButton) && !pressed) 
        {
            const auto pos = inputHandler->getMousePos();

            std::cout << this->pos.x - pos.x() << " " << this->pos.y - pos.y() << std::endl;
            
            pressed = true;
        } 

        if(!inputHandler->isButtonPressed(Qt::LeftButton)) 
            pressed = false;
    }
    
}