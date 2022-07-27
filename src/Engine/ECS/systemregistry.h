#pragma once

#include <memory>

namespace pg
{
    namespace ecs
    {
        class SystemRegistry
        {
        public:
            static std::unique_ptr<SystemRegistry>& getRegistry()
            {
                static std::unique_ptr<SystemRegistry> registry = std::unique_ptr<SystemRegistry>(new SystemRegistry());
                return registry;
            }

            template<class Sys>
            void registerSystem();

            template<class Sys>
            Sys* getSystem();

        private:
            std::vector<AbstractSystem*> systemRegistry;
        };
    }
}