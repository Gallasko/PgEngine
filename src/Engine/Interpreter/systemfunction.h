#pragma once

#include "environment.h"

namespace pg
{

    class ToString : public Function
    {
        using Function::Function;
    public:
        void setUp();

        virtual ValuablePtr call(ValuableQueue& args) override;
    };

    class LogInfo : public Function
    {
        using Function::Function;
    public:
        void setUp();

        virtual ValuablePtr call(ValuableQueue& args) override;
    };

    class HRClock : public Function
    {
        using Function::Function;
    public:
        void setUp();

        virtual ValuablePtr call(ValuableQueue& args) override;
    };

}
