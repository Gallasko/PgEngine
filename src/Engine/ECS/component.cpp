#include "component.h"

#include <iostream>

namespace pg
{
    namespace ecs
    {
        Component::Component(const std::string& name)
        {
            this->name = name;
            //std::cout << "Creating component: " << name << std::endl;
        }
    }
}