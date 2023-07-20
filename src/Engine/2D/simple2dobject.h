#pragma once

#include "logger.h"

namespace pg
{
    class Simple2DObject
    {
    public:
        enum class Type : uint8_t
        {
            Circle,
            Square,
            Triangle,
            None
        };

    public:
        virtual Type getType() const = 0;  

    protected:
    
    };

    class SimpleCircle : public Simple2DObject
    {
    public:
        virtual Type getType() const override { return Type::Circle; }

    protected:
    };

    class SimpleSquare : public Simple2DObject
    {
    public:
        virtual Type getType() const override { return Type::Square; }

    };

    class SimpleTriangle : public Simple2DObject
    {
    public:
        virtual Type getType() const override { return Type::Triangle; }

    };

}