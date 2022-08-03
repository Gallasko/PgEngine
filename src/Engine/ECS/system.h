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
        /**
        struct OwnedComponent
        {
            template <typename Comp, typename... Args>
            Component* create(const Args&... args);

            void remove(Component* component);

            std::vector<Component* > components;
        };

        template <typename Type>
        struct Own : public OwnedComponents
        {
            Own(){}
        };

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

            template<typename... Args>
            Component* createComponent(const std::string& name, const Args&... args) const { return creationMap.at(name)(name, args...); }

            virtual void execute() = 0;

        protected:
            std::unordered_map<std::string, componentCreateFunction> creationMap;
        };

        void addCreationFunction(std::unordered_map<std::string, componentCreateFunction> *cTorLookupTable);

        template <class... B>
        typename std::enable_if<sizeof...(B) == 0>::type addCreationFunction(std::unordered_map<std::string, componentCreateFunction> *cTorLookupTable)
        {
            // Does nothing, terminator class for addCreationFunction
        }

        template <class A, class... B, class... Args> void addCreationFunction(std::unordered_map<std::string, componentCreateFunction> *cTorLookupTable)
        {
            // TODO: lookup for A name in ECS name system
            (*cTorLookupTable)["temp"] = [](const std::string& name, const Args&... args){ return new A(args...); };
            addCreationFunction<B... >(cTorLookupTable);
        }

        template<typename... Comps>
        struct System : public AbstractSystem
        {
            System()
            {
                addCreationFunction<Comps...>(&(this->creationMap));
            }
        };
    }
}