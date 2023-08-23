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

        }

        virtual void addToRegistry(ComponentRegistry *registry) override
        {
            LOG_THIS_MEMBER("System");

            this->registry = registry;

            this->ecsRef = registry->world();

            auto sysMethods = sysInstance->getMethods();
            auto sysFields = sysInstance->getFields();

            const auto nameIt = sysFields.find("name");

            if(nameIt != sysFields.end())
                this->name = nameIt->second->getValue()->getElement().toString();


        }

        void onEvent(_unique_id id, std::shared_ptr<ClassInstance> values)
        {

        }

    private:
        std::shared_ptr<Environment> env;
        std::shared_ptr<ClassInstance> sysInstance;
    };

}