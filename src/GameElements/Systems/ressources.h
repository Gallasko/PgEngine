#pragma once

#include "../../Engine/constant.h"

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