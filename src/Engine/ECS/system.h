#pragma once

#include <string>
#include <vector>

#include "component.h"
#include "uniqueid.h"
#include "componentregistry.h"

namespace pg
{
    namespace ecs
    {
        // typedef Component*(*componentCreateFunction)(const std::string&, ...);

        struct AbstractSystem
        {
            virtual ~AbstractSystem() {}

            virtual void execute() = 0;
        };

        enum class Ownership
        {
            NONE = 0,
            OWNED,
            REFFERED
        };

        typedef std::unordered_map<std::string, Ownership> OwnershipMap;

        template <typename Comp, typename... Comps>
        struct System;

        template <class Owner, class B, typename... Comps>
        void addOwnershipMap(System<Comps...> *system, tag<Owner>, OwnershipMap *ownershipMap);

        template <class A, typename... Comps>
        void addOwnershipMap(System<Comps...> *system, tag<Own<A>>, OwnershipMap *ownershipMap)
        {
            (*ownershipMap)[typeid(A).name()] = Ownership::OWNED;
        }

        template <class A, typename... Comps>
        void addOwnershipMap(System<Comps...> *system, tag<Ref<A>>, OwnershipMap *ownershipMap)
        {
            (*ownershipMap)[typeid(A).name()] = Ownership::REFFERED;
        }

        template <class Owner, class A, class B, class... C, typename... Comps>
        void addOwnershipMap(System<Comps...> *system, tag<Owner>, OwnershipMap *ownershipMap);

        template <class A, class B, class... C,  typename... Comps>
        void addOwnershipMap(System<Comps...> *system, tag<Own<A>>, OwnershipMap *ownershipMap)
        {
            // TODO: lookup for A name in ECS name system
            (*ownershipMap)[typeid(A).name()] = Ownership::OWNED;
            addOwnershipMap<B, C...>(system, tag<B>{}, ownershipMap);
        }

        template <class A, class B, class... C,  typename... Comps>
        void addOwnershipMap(System<Comps...> *system, tag<Ref<A>>, OwnershipMap *ownershipMap)
        {
            // TODO: lookup for A name in ECS name system
            (*ownershipMap)[typeid(A).name()] = Ownership::REFFERED;
            addOwnershipMap<B, C...>(system, tag<B>{}, ownershipMap);
        }

        template <typename Comp, typename... Comps>
        struct System : public AbstractSystem, public Comp, public Comps...
        {
            System()
            {
                addOwnershipMap<Comp, Comps...>(this, tag<Comp>{}, &(this->ownershipMap));
            }

            template <typename Type, typename... Args>
            Type* createComponent(_entityId id, const Args&... args)
            {
                switch(ownershipMap[typeid(Type).name()])
                {
                    case Ownership::OWNED:
                        return static_cast<Own<Type>*>(this)->internalCreateComponent(id, args...);
                        break;
                    case Ownership::REFFERED:
                        return static_cast<Ref<Type>*>(this)->internalCreateComponent(id, args...);
                        break;
                    
                    default:
                        // LOG_ERROR;
                        return nullptr;
                }
            }

            OwnershipMap ownershipMap;
        };
    }
}