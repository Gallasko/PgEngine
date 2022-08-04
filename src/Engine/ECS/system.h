#pragma once

#include <string>
#include <vector>

#include "component.h"
#include "uniqueid.h"
#include "componentregistry.h"

#include <iostream>

namespace pg
{
    namespace ecs
    {
        // typedef Component*(*componentCreateFunction)(const std::string&, ...);

        enum class Ownership
        {
            NONE = 0,
            OWNED,
            REFFERED
        };

        typedef std::unordered_map<std::string, Ownership> OwnershipMap;

        struct AbstractSystem
        {
            virtual ~AbstractSystem() {}

            virtual void execute() = 0;

            OwnershipMap ownershipMap;
        };

        template <typename... Comps>
        struct System;

        template <typename Sys>
        void setOwnershipMap(Sys*, ComponentRegistry*, OwnershipMap*)
        {
            std::cout << "Nothing to see here." << std::endl;
        }

        template <typename Comp, typename... Comps, typename Sys>
        void setOwnershipMap(Sys *system, ComponentRegistry *registry, OwnershipMap *ownershipMap, const tag<Own<Comp>>&, const Comps&... comps)
        {
            static_cast<Own<Comp>*>(system)->setRegistry(registry);
            setOwnershipMap(system, registry, ownershipMap, comps...);
        }

        template <typename Comp, typename... Comps, typename Sys>
        void setOwnershipMap(Sys *system, ComponentRegistry *registry, OwnershipMap *ownershipMap, const tag<Ref<Comp>>&, const Comps&... comps)
        {
            static_cast<Ref<Comp>*>(system)->setRegistry(registry);
            setOwnershipMap(system, registry, ownershipMap, comps...);
        }

        template <typename... Comps>
        struct System : public AbstractSystem, public Comps...
        {
            System() : AbstractSystem(), Comps()...
            {
            }

            void setRegistry(ComponentRegistry *registry)
            {
                setOwnershipMap(this, registry, &(this->ownershipMap), tag<Comps>{}...);
            }

            template <typename Type, typename... Args>
            Type* createComponent(_entityId id, const Args&... args)
            {
                return this->Ref<Type>::internalCreateComponent(id, args...);
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

        };
    }
}