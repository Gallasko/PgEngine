#pragma once

#include "componentregistry.h"
#include "system.h"

namespace pg
{
    namespace ecs
    {
        class EntitySystem
        {
        public:
            template<class Sys, typename... Args>
            Sys* createSystem(const Args&... args)
            {
                auto system = new Sys(args...);
                system->setRegistry(&registry);

                systems.push_back(system);

                return system;
            }

        private:
            ComponentRegistry registry;

            std::vector<AbstractSystem*> systems;
        };
    }
}