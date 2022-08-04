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
/*
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
*/

        template <typename Comp>
        using Owned = Own<Comp>;

        template <typename Comp>
        using Reffered = Ref<Comp>;

        template <typename Sys>
        void test(Sys *system, OwnershipMap *ownershipMap)
        {
            std::cout << "Nothing to see here." << std::endl;
        }

        template <template<class> class Owner, typename Comp, typename Sys>
        void test_test(Sys *system, OwnershipMap *ownershipMap)
        {
            std::cout << typeid(Owner<Comp>).name() << std::endl;
            std::cout << "End" << std::endl;
        }

        template <typename Comp, typename... Comps, typename Sys>
        void test(Sys *system, OwnershipMap *ownershipMap)
        {
            std::cout << typeid(Owned<Comp>).name() << std::endl;
            test<Comps...>(system, ownershipMap);
        }

        /*
        template <typename Comp, typename... Comps, typename Sys>
        void test(Sys *system, OwnershipMap *ownershipMap)
        {
            std::cout << typeid(Referred<Comp>).name() << std::endl;
            test<Comps...>(system, ownershipMap);
        }
        */

        template <typename... Comps>
        struct System : public AbstractSystem, public Comps...
        {
            System()
            {
                test<Comps...>(this, &(this->ownershipMap));
            }

/*
            template <typename Type, typename... Args>
            Type* createComponent(_entityId id, const Args&... args)
            {
                switch(ownershipMap[typeid(Type).name()])
                {
                    case Ownership::OWNED:
                        return createOwnedComponent(id, args...);
                        break;
                    case Ownership::REFFERED:
                        return createRefferedComponent(id, args...);
                        break;
                    
                    default:
                        // LOG_ERROR;
                        return nullptr;
                }
            }
*/

            template <typename Type, typename... Args>
            Type* createOwnedComponent(_entityId id, const Args&... args)
            {
                return static_cast<Own<Type>*>(this)->internalCreateComponent(id, args...);
            }

            template <typename Type, typename... Args>
            Type* createRefferedComponent(_entityId id, const Args&... args)
            {
                return static_cast<Ref<Type>*>(this)->internalCreateComponent(id, args...);
            }

        };
    }
}