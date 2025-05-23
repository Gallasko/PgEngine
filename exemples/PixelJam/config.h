#pragma once

#include "Input/keyconfig.h"

namespace pg
{
    enum class GameKeyConfig : uint8_t
    {
        MoveLeft,
        MoveRight,
        MoveUp,
        MoveDown,
        Interact,
        Pause,
        Dodge,
        Heal
    };

    extern std::map<GameKeyConfig, DefaultScancode> scancodeMap;

} // namespace pg
