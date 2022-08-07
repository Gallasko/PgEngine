#pragma once

#include <string>
#include <unordered_map>

#include "entity.h"
#include "component.h"
#include "componentregistry.h"

#include "logger.h"

namespace pg
{
    namespace ecs
    {
        /**
         * @brief Abstract representation of a system
         */
        struct AbstractSystem
        {
            virtual ~AbstractSystem() {}

            virtual void execute() = 0;
            
            // Todo make function onAdd and onDelete of a component that default to nothing if not used
        };

        template <typename... Comps>
        struct System;

        template <typename Sys>
        void registerComponents(Sys*, ComponentRegistry*) {}

        template <typename Comp, typename... Comps, typename Sys>
        void registerComponents(Sys *system, ComponentRegistry *registry, const tag<Own<Comp>>&, const Comps&... comps)
        {
            static_cast<Own<Comp>*>(system)->setRegistry(registry);
            registerComponents(system, registry, comps...);
        }

        template <typename Comp, typename... Comps, typename Sys>
        void registerComponents(Sys *system, ComponentRegistry *registry, const tag<Ref<Comp>>&, const Comps&... comps)
        {
            static_cast<Ref<Comp>*>(system)->setRegistry(registry);
            registerComponents(system, registry, comps...);
        }

        template <typename... Comps>
        struct System : public AbstractSystem, public Comps...
        {
            System() : AbstractSystem(), Comps(generateId())...
            {
                LOG_THIS_MEMBER("System");
                System<Comps...>::systemid = generateId();
            }

            ~System()
            {
                LOG_THIS_MEMBER("System");
            }

            void setRegistry(ComponentRegistry *registry)
            {
                LOG_THIS_MEMBER("System");

                registerComponents(this, registry, tag<Comps>{}...);
            }

            template <typename Type, typename... Args>
            Type* createComponent(const Entity& entity, const Args&... args)
            {
                LOG_THIS_MEMBER("System");

                return this->createRefferedComponent<Type>(entity, args...);
            }

            template <typename Type, typename... Args>
            Type* createOwnedComponent(const Entity& entity, const Args&... args)
            {
                LOG_THIS_MEMBER("System");

                return this->Own<Type>::internalCreateComponent(entity, args...);
            }

            template <typename Type, typename... Args>
            Type* createRefferedComponent(const Entity& entity, const Args&... args)
            {
                LOG_THIS_MEMBER("System");

                return this->Ref<Type>::internalCreateComponent(entity, args...);
            }

            template <typename Type>
            SparseSet::SparseSetList<Type> view() const
            {
                LOG_THIS_MEMBER("System");

                return this->Ref<Type>::view();
            }

            static _unique_id systemid;
        };

        template <typename... Comps>
        _unique_id System<Comps...>::systemid = 0;
    }
}