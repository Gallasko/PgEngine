#include "listview.h"

namespace pg
{
    SlideBar::SlideBar(const UiFrame& frame, UiSize* posToUpdate)
    {
        this->pos.x = &frame.x;
        this->pos.y = &frame.y;

        this->width = &frame.w;
        this->height = &frame.h;

        posUpdate = posToUpdate;
    }
    
}