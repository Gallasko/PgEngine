#pragma once

#include "interpreter.h"

#include "ECS/system.h"

namespace pg
{
    class InterpreterSystem : public System<>
    {
    public:
        InterpreterSystem(std::shared_ptr<Environment> env, std::shared_ptr<ClassInstance> sysInstance) : env(env), sysInstance(sysInstance)
        {
            LOG_THIS_MEMBER("Interpreter System");
        }

        virtual void addToRegistry(ComponentRegistry *registry) override
        {
            LOG_THIS_MEMBER("Interpreter System");

            this->registry = registry;

            this->ecsRef = registry->world();

            auto sysMethods = sysInstance->getMethods();
            auto sysFields = sysInstance->getFields();

            const auto& nameIt = sysFields.find("name");

            if(nameIt != sysFields.end())
                this->name = nameIt->second->getValue()->getElement().toString();

            const auto& policyIt = sysFields.find("policy");

            if(policyIt != sysFields.end())
            {
                const auto policy = policyIt->second->getValue()->getElement().toString();

                if(policy == "manual")
                {
                    this->executionPolicy = ExecutionPolicy::Manual;
                }
                else if(policy == "sequential")
                {
                    this->executionPolicy = ExecutionPolicy::Sequential;
                }
                else if(policy == "parallel")
                {
                    this->executionPolicy = ExecutionPolicy::Parallel;
                }
                else if(policy == "independent")
                {
                    this->executionPolicy = ExecutionPolicy::Independent;
                }
                else if(policy == "storage")
                {
                    this->executionPolicy = ExecutionPolicy::Storage;
                }
            }

            if(executionPolicy == ExecutionPolicy::Independent or executionPolicy == ExecutionPolicy::Sequential)
            {
                //Todo also check for a sysfield name "execute" !
                const auto& executeIt = sysMethods.find("execute");

                if(executeIt != sysMethods.end())
                {
                    // Todo check for the arity of the function when registering
                    if(executeIt->second->getType() == "Function")
                    {
                        executeMethod = std::static_pointer_cast<Function>(executeIt->second);
                    }
                    else
                    {
                        throw RuntimeException(Token{TokenType::EXPRESSION, executeIt->second->getElement().toString(), 0, 0}, "Method execute is not a function for an executable system");
                    }
                }
                else
                {
                    throw RuntimeException(Token{TokenType::EXPRESSION, sysInstance->getElement().toString(), 0, 0}, "No execute method found for an executable system");
                }
            }
            
        }

        virtual void execute() override
        {
            LOG_THIS_MEMBER("Interpreter System");

            if(not executeMethod)
                return;

            ValuableQueue emptyQueue;
            
            try
            {    
                auto m = executeMethod->getVisitor()->getMutex();
                m->lock();
                executeMethod->getValue(emptyQueue);
                m->unlock();
            }
            catch(const std::exception& e)
            {
                LOG_ERROR("Interpreter System", e.what());
            }
        }

        void onEvent(_unique_id id, std::shared_ptr<ClassInstance> values)
        {
            LOG_THIS_MEMBER("Interpreter System");
        }

    private:
        std::shared_ptr<Environment> env;
        std::shared_ptr<ClassInstance> sysInstance;
        std::shared_ptr<Function> executeMethod;
    };

}