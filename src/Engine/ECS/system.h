#pragma once

#include <string>
#include <unordered_map>

#include "component.h"
#include "componentregistry.h"

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
        void setOwnershipMap(Sys*, ComponentRegistry*) {}

        template <typename Comp, typename... Comps, typename Sys>
        void setOwnershipMap(Sys *system, ComponentRegistry *registry, const tag<Own<Comp>>&, const Comps&... comps)
        {
            static_cast<Own<Comp>*>(system)->setRegistry(registry);
            setOwnershipMap(system, registry, comps...);
        }

        template <typename Comp, typename... Comps, typename Sys>
        void setOwnershipMap(Sys *system, ComponentRegistry *registry, const tag<Ref<Comp>>&, const Comps&... comps)
        {
            static_cast<Ref<Comp>*>(system)->setRegistry(registry);
            setOwnershipMap(system, registry, comps...);
        }

        template <typename... Comps>
        struct System : public AbstractSystem, public Comps...
        {
            System() : AbstractSystem(), Comps()...
            {
            }

            void setRegistry(ComponentRegistry *registry)
            {
                setOwnershipMap(this, registry, tag<Comps>{}...);
            }

            template <typename Type, typename... Args>
            Type* createComponent(_entityId id, const Args&... args)
            {
                return this->createRefferedComponent<Type>(id, args...);
            }

            template <typename Type, typename... Args>
            Type* createOwnedComponent(_entityId id, const Args&... args)
            {
                return this->Own<Type>::internalCreateComponent(id, args...);
            }

            template <typename Type, typename... Args>
            Type* createRefferedComponent(_entityId id, const Args&... args)
            {
                return this->Ref<Type>::internalCreateComponent(id, args...);
            }

            template <typename Type>
            SparseSet::SparseSetList<Type> view() const
            {
                return this->Ref<Type>::view();
            }

        // Todo have access to a view of the component
        };
    }
}