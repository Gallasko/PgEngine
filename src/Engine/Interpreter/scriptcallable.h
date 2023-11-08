#pragma once

#include "ECS/callable.h"

#include "interpreter.h"

namespace pg
{
    /**
     * @brief Redefinition of AbstractCallable that execute function of scripts when an action is executed
     */
    struct CallableIntepretedFunction : public AbstractCallable
    {
        /**
         * @brief Construct a new Callable Intepreted Function object
         * 
         * @param fun The function to execute
         * 
         * When contructing this object we need to make the function independant of the rest of the script
         * to avoid any conflict of environment during runtime of multiple functions comming from the same script.
         */
        CallableIntepretedFunction(std::shared_ptr<Function> fun)
        {
            // Mark the function script as part of the ECS
            fun->getVisitor()->setEcsSysFlag();

            // Create an independant visitor to make the function independent
            visitorRef = fun->getVisitor()->getVisitorRef();

            // Copy the content of the function 
            function = std::make_shared<Function>(visitorRef, fun);
        }

        virtual ~CallableIntepretedFunction() {}

        /**
         * @brief Execute the registered function without arguments
         */
        inline virtual void call(EntitySystem* const) noexcept override
        {
            ValuableQueue emptyQueue;

            try
            {
                function->getValue(emptyQueue);
            }
            catch (const std::exception& e)
            {
                LOG_ERROR("CallableIntepretedFunction", e.what() << " in script: " << visitorRef->getScriptName());
            }
        }

        /**
         * @brief Execute the registered function with arguments
         */
        inline void call(std::queue<std::shared_ptr<Valuable>>& args) noexcept
        {
            try
            {
                function->getValue(args);
            }
            catch (const std::exception& e)
            {
                LOG_ERROR("CallableIntepretedFunction", e.what() << " in script: " << visitorRef->getScriptName());
            }
        }

        inline virtual void serialize(Archive& archive) const noexcept override
        {
            archive.startSerialization("CallableEvent");

            pg::serialize(archive, "script", visitorRef->getScriptName());

            archive.endSerialization();
        }

        /**
         * @brief Get the underlying Function pointer
         * 
         * @return Function* A pointer to the registered function
         */
        inline Function* getRef() const { return function.get(); }

        /** Registered function to call */
        std::shared_ptr<Function> function;

        /** A reference visitor to the main one allowing for multiple functions comming from the same script */
        std::shared_ptr<VisitorReference> visitorRef;
    };

    /**
     * @brief Helper function used to create a CallableIntepretedFunction pointer
     * 
     * @param fun The function to use
     * 
     * @return std::shared_ptr<CallableIntepretedFunction> A pointer to the callable object
     */
    std::shared_ptr<CallableIntepretedFunction> makeCallable(std::shared_ptr<Function> fun);
}