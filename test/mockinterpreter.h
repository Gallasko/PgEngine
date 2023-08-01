#include "gtest/gtest.h"

#include "Interpreter/pginterpreter.h"

namespace pg
{
    class ExpectEq : public Function
    {
        using Function::Function;
    public:
        void setUp()
        {
            setArity(2, 2);
        }

        virtual ValuablePtr call(ValuableQueue& args) const override
        {
            auto v1 = args.front()->getElement();
            args.pop();

            auto v2 = args.front()->getElement();
            args.pop();

            EXPECT_TRUE((v1 == v2).isTrue());

            return nullptr;
        }
    };

    class ExpectTrue : public Function
    {
        using Function::Function;
    public:
        void setUp()
        {
            setArity(1, 1);
        }

        virtual ValuablePtr call(ValuableQueue& args) const override
        {
            auto v = args.front()->getElement();
            args.pop();

            EXPECT_TRUE(v.isTrue());

            return nullptr;
        }
    };

    class TestPrint : public Function
    {
        using Function::Function;
    public:
        void setUp()
        {
            setArity(1, 1);
        }

        virtual ValuablePtr call(ValuableQueue& args) const override
        {
            auto v = args.front()->getElement();
            args.pop();

            std::cout << "[Interpreter]: " << v.toString() << std::endl;

            return nullptr;
        }
    };

    class MockInterpreter : public PgInterpreter
    {
    public:
        MockInterpreter() : PgInterpreter()
        {
            this->addSystemFunction<ExpectTrue>("ExpectTrue");
            this->addSystemFunction<ExpectEq>("ExpectEq");

            this->addSystemFunction<TestPrint>("print");
        }
    };
}


