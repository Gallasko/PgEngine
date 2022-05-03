#pragma once

#include "../../Engine/UI/uisystem.h"

using namespace pg;

struct EscapePanel : public UiComponent
{
    TextureComponent *background;

    EscapePanel(TextureComponent *background = nullptr);

    virtual void show();
    virtual void hide();
};