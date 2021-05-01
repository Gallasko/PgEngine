#include "uisystem.h"

void UiComponent::update()
{
    if(topAnchor != nullptr && bottomAnchor != nullptr)
    {
        this->height = bottomAnchor->y - bottomMargin - topAnchor->y - topAnchor->height - topMargin;
        this->y = topAnchor->y + topAnchor->height + topMargin;
    }
    else if(topAnchor != nullptr && bottomAnchor == nullptr)
    {
        this->y = topAnchor->y + topAnchor->height + topMargin;
    }
    else if(topAnchor == nullptr && bottomAnchor != nullptr)
    {
        this->y = bottomAnchor->y - height - bottomMargin;
    }

    if(rightAnchor != nullptr && leftAnchor != nullptr)
    {
        this->width = rightAnchor->x - rightMargin - leftAnchor->x - leftAnchor->width - leftMargin;
        this->x = leftAnchor->x + leftAnchor->width + leftMargin;
    }
    else if(rightAnchor != nullptr && leftAnchor == nullptr)
    {
        this->x = rightAnchor->x - width - rightMargin;
    }
    else if(rightAnchor == nullptr && leftAnchor != nullptr)
    {
        this->x = leftAnchor->x + leftAnchor->width + leftMargin;
    }

    for(auto child : children)
        child->update();
}