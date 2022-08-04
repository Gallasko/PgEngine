#pragma once

/*
#include "componentregistry.h"
#include "system.h"

namespace pg
{
    namespace ecs
    {
        class EntitySystem
        {
        public:

            template<class Sys, template<typename> class... Owner, typename... Args, typename Comp>
            void createSystem(const Args&... args)
            {
                auto system = new Sys(args...);

                system->registerComponents<Owner<Comp>...>(&registry);
            }

        private:
            ComponentRegistry registry;
        };
    }
}
*/