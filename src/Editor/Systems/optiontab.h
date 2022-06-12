#pragma once

#include "UI/uisystem.h"

namespace pg
{

namespace editor
{
    class OptionTab;

    template <typename T>
    void setOption(OptionTab *tab, T* value);

    class OptionTab : public UiComponent
    {

    }; 
}
}