#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "component.h"
#include "uniqueid.h"


namespace pg
{
    namespace ecs
    {

        struct OwnedComponent
        {
            template <typename Comp, typename... Args>
            Component* create(const Args&... args);

            void remove(Component* component);

            std::vector<Component* > components;
        };

        template<typename Type>
        struct Own
        {
            template <typename... Args>
            Type* internalCreateComponent(const Args&... args)
            {
                return new Type(args...);
            }
        };

        /**
        struct ComponentHolder
        {

        };

        template <typename Type>
        struct Ref : public ComponentHolder
        {

        };
        */

        typedef Component*(*componentCreateFunction)(const std::string&, ...);

        struct AbstractSystem
        {
            virtual ~AbstractSystem() {}

            //template<typename... Args>
            //Component* createComponent(const std::string& name, const Args&... args) const { return creationMap.at(name)(name, args...); }

            virtual void execute() = 0;

        protected:
            //std::unordered_map<std::string, componentCreateFunction> creationMap;
        };

        /*
        template <typename... Comps>
        struct System;

        void addCreationFunction(System<> *system, std::unordered_map<std::string, componentCreateFunction> *cTorLookupTable)
        {
            // Does nothing, terminator class for addCreationFunction
        }

        template <class... B, typename... Comps>
        typename std::enable_if<sizeof...(B) == 0>::type addCreationFunction(System<Comps...> *system, std::unordered_map<std::string, componentCreateFunction> *cTorLookupTable)
        {
            // Does nothing, terminator class for addCreationFunction
        }

        template <class A, class... B, typename... Comps>
        void addCreationFunction(System<Comps...> *system, std::unordered_map<std::string, componentCreateFunction> *cTorLookupTable)
        {
            // TODO: lookup for A name in ECS name system
            (*cTorLookupTable)["temp"] = [system](const std::string &name, ...){ return static_cast<A*>(system)->internalCreateComponent(name); };
            addCreationFunction<B... >(system, cTorLookupTable);
        }
        */

        template <typename... Comps>
        struct System : public AbstractSystem, public Comps...
        {
            template <typename Type, typename... Args>
            Type* createComponent(const Args&... args) { return static_cast<Own<Type>*>(this)->internalCreateComponent(args...); }
            /*
            System()
            {
                addCreationFunction<Comps...>(this, &(this->creationMap));
            }
            */
        };
    }
}