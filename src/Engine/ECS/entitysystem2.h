#pragma once

#include "uniqueid.h"
#include "componentregistry.h"
#include "system.h"

namespace pg
{
    namespace ecs
    {
        class EntitySystem
        {
        private:
            struct Entity
            {
                Entity(_entityId id) : id(id) {}

                _entityId id;
            };
        public:
            Entity createEntity() const
            {
                Entity entity(generateId());

                return entity;
            }

            template<class Sys, typename... Args>
            Sys* createSystem(const Args&... args)
            {
                auto system = new Sys(args...);
                system->setRegistry(&registry);

                systems.push_back(system);

                return system;
            }

            template<typename Type, typename... Args>
            Type* attach(const Entity& entity, const Args&... args) const
            {
                return registry.retrieve<Type>()->internalCreateComponent(entity.id, args...);
            }

        private:
            ComponentRegistry registry;

            std::vector<AbstractSystem*> systems;
        };
    }
}