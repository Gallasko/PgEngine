#pragma once

#include "environment.h"

class ToString : public Function
{
    using Function::Function;
public:
    virtual void setUp() override;

    virtual ValuablePtr call(ValuableQueue& args) const override;
};

class LogInfo : public Function
{
    using Function::Function;
public:
    virtual void setUp() override;

    virtual ValuablePtr call(ValuableQueue& args) const override;
};

class HRClock : public Function
{
    using Function::Function;
public:
    virtual void setUp() override;

    virtual ValuablePtr call(ValuableQueue& args) const override;
};
