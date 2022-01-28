#include "listview.h"

namespace pg
{
    template<>
    void renderer(MasterRenderer* masterRenderer, SlideBar* slideBar)
    {
        masterRenderer->render(slideBar->slider);
        masterRenderer->render(slideBar->cursor);
    }

    SlideBar::SlideBar(const UiFrame& frame, UiSize* posToUpdate)
    {
        this->pos.x = &frame.x;
        this->pos.y = &frame.y;

        this->width = &frame.w;
        this->height = &frame.h;

        posUpdate = posToUpdate;
    }

    void mouseInput(Input* inputHandler, double deltaTime...)
    {
        static bool pressed = false;

        if(inputHandler->isButtonPressed(Qt::LeftButton) && !pressed) 
        {
            
            
            pressed = true;
        } 

        if(!inputHandler->isButtonPressed(Qt::LeftButton)) 
            pressed = false;
    }
    
}