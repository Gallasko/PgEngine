#pragma once

#include "../../constant.h"

#include <string>

struct Ressources
{
    std::string name;
    pg::constant::BigInt value;
};

class Effect
{
public:
    enum class EffectType
    {
        INCREASE,
        GENERATE
    };
};