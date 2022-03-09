#pragma once

#include "../../UI/uisystem.h"

using namespace pg;

struct EscapePanel : public UiComponent
{
    TextureComponent* background;

    EscapePanel(TextureComponent* background = nullptr);
};