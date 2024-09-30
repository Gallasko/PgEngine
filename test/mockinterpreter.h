#include "gtest/gtest.h"

#include "Interpreter/pginterpreter.h"
#include "Interpreter/systemfunction.h"

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

        virtual ValuablePtr call(ValuableQueue& args) override
        {
            auto v1 = args.front()->getElement();
            args.pop();

            auto v2 = args.front()->getElement();
            args.pop();

            EXPECT_TRUE((v1 == v2).isTrue());

            if (not (v1 == v2).isTrue())
            {
                std::cout << "Expected: " << v1.toString() <<  ", Got: " << v2.toString() << " at line: " << this->getToken().line << std::endl;
            }

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

        virtual ValuablePtr call(ValuableQueue& args) override
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

        virtual ValuablePtr call(ValuableQueue& args) override
        {
            auto v = args.front()->getElement();
            args.pop();

            // std::cout << "[Interpreter]: " << v.toString() << std::endl;

            return nullptr;
        }
    };

    class MethodPrint : public Function
    {
        using Function::Function;
    public:
        void setUp()
        {
            setArity(1, 1);
        }

        virtual ValuablePtr call(ValuableQueue& args) override
        {
            auto v = args.front();
            args.pop();

            if (v->getType() == "ClassInstance")
            {
                auto obj = std::static_pointer_cast<ClassInstance>(v);

                auto methods = obj->getMethods();

                // std::cout << "[Interpreter]: Methods of " << obj->getElement().toString() << std::endl;

                // for(auto method : methods)
                // {
                //     std::cout << "[Interpreter]: " << method.first << std::endl;
                // }
            }
            else if (v->getType() == "IteratorInstance")
            {
                auto obj = std::static_pointer_cast<IteratorInstance>(v);

                auto methods = obj->getMethods();

                // std::cout << "[Interpreter]: Methods of " << obj->getElement().toString() << std::endl;

                // for(auto method : methods)
                // {
                //     std::cout << "[Interpreter]: " << method.first << std::endl;
                // }
            }

            return nullptr;
        }
    };

    class PostExecutionCallback : public Function
    {
        using Function::Function;
    public:
        void setUp(std::function<void()> *callback)
        {
            func = callback;
            setArity(1, 1);
        }

        virtual ValuablePtr call(ValuableQueue& args) override
        {
            auto arg = args.front();
            args.pop();

            std::cout << arg->getType() << std::endl;

            if (arg->getType() == "Function")
            {
                auto fun = std::static_pointer_cast<Function>(arg);

                std::cout << "[PostExecutionCallback]: " << std::endl;

                *func = [fun](){
                    ValuableQueue queue;

                    fun->getValue(queue);
                };

                visitor->setEcsSysFlag();
            }

            return nullptr;
        }

        std::function<void()> *func;
    };

    class MockInterpreter : public PgInterpreter
    {
    public:
        MockInterpreter() : PgInterpreter()
        {
            addSystemFunction<ExpectTrue>("ExpectTrue");
            addSystemFunction<ExpectEq>("ExpectEq");

            addSystemFunction<TestPrint>("print");
            addSystemFunction<LogInfo>("logInfo");
            addSystemFunction<MethodPrint>("mPrint");
            addSystemFunction<ToString>("toString");
        }
    };
}


