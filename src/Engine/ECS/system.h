#pragma once

#include <string>
#include <unordered_map>

#include "entity.h"
#include "component.h"
#include "componentregistry.h"
#include "group.h"

#include "logger.h"

namespace pg
{
    namespace ecs
    {
        enum class ExecutionPolicy
        {
            Manual      = 0,
            Sequential  = 1,
            Parallel    = 2,
            Independent = 3,
            OnEvent     = 4,
            Storage     = 5
        };

        /**
         * @brief Abstract representation of a system
         */
        struct AbstractSystem
        {
            virtual ~AbstractSystem() { LOG_THIS_MEMBER("System"); }

            virtual void execute() { LOG_THIS_MEMBER("System"); }

            virtual void parallelExecute(size_t start, size_t end) { LOG_THIS_MEMBER("System"); }

            inline void setPolicy(const ExecutionPolicy& policy) { executionPolicy = policy; }

            ExecutionPolicy executionPolicy = ExecutionPolicy::Manual;

            ComponentRegistry *registry = nullptr;

            _unique_id id;

            // Todo make function onAdd and onDelete of a component that default to nothing if not used
        };

        template <typename... Comps>
        struct System;

        template <typename Sys>
        void registerComponents(Sys*, ComponentRegistry*) { LOG_THIS("System"); }

        template <typename Comp, typename... Comps, typename Sys>
        void registerComponents(Sys *system, ComponentRegistry *registry, const tag<Own<Comp>>&, const Comps&... comps)
        {
            LOG_THIS("System");

            LOG_INFO("System", "Registering an own to '" + std::string(typeid(Comp).name()) + "' to the system.");

            static_cast<Own<Comp>*>(system)->setRegistry(registry);
            registerComponents(system, registry, comps...);
        }

        template <typename Comp, typename... Comps, typename Sys>
        void registerComponents(Sys *system, ComponentRegistry *registry, const tag<Ref<Comp>>&, const Comps&... comps)
        {
            LOG_THIS("System");
            
            LOG_INFO("System", "Registering a ref to '" + std::string(typeid(Comp).name()) + "' to the system.");
            
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
                id = System<Comps...>::systemid;
            }

            ~System()
            {
                LOG_THIS_MEMBER("System");
            }

            void setRegistry(ComponentRegistry *registry)
            {
                LOG_THIS_MEMBER("System");

                this->registry = registry;

                registerComponents(this, registry, tag<Comps>{}...);
            }

            template <typename Type, typename... Args>
            inline Type* createComponent(Entity* entity, Args&&... args)
            {
                LOG_THIS_MEMBER("System");

                return this->createRefferedComponent<Type>(entity, std::forward<Args>(args)...);
            }

            template <typename Type, typename... Args>
            inline Type* createOwnedComponent(Entity* entity, Args&&... args)
            {
                LOG_THIS_MEMBER("System");

                return this->Own<Type>::internalCreateComponent(entity, std::forward<Args>(args)...);
            }

            template <typename Type, typename... Args>
            inline Type* createRefferedComponent(Entity* entity, Args&&... args)
            {
                LOG_THIS_MEMBER("System");

                return this->Ref<Type>::internalCreateComponent(entity, std::forward<Args>(args)...);
            }

            template <typename Type>
            inline void removeComponent(Entity* entity)
            {
                LOG_THIS_MEMBER("System");

                this->removeRefferedComponent(entity);
            }

            template <typename Type>
            void removeRefferedComponent(Entity* entity)
            {
                LOG_THIS_MEMBER("System");

                this->Ref<Type>::internalRemoveComponent(entity);
            }

            template <typename Type>
            inline void removeOwnedComponent(Entity* entity)
            {
                LOG_THIS_MEMBER("System");

                this->Own<Type>::internalRemoveComponent(entity);
            }

            template <typename Type>
            inline typename ComponentSet<Type>::ComponentSetList view() const
            {
                LOG_THIS_MEMBER("System");

                return this->Ref<Type>::view();
            }

            template <typename Type, typename... Types>
            const Group<Type, Types...>* group() const
            {
                LOG_THIS_MEMBER("System");

                if(registry == nullptr)
                {
                    LOG_ERROR("System", "No registry specified, can't create a group");
                    return nullptr;
                }

                if(Group<Type, Types...>::groupId != 0)
                    return registry->retrieveGroup<Type, Types...>();
                else
                {
                    LOG_INFO("System", "Creating new group");
                    auto group = new Group<Type, Types...>(generateId());
                    
                    group->setRegistry(registry);
                    group->process();

                    return group;
                }

            }

            static _unique_id systemid;
        };

        template <typename... Comps>
        _unique_id System<Comps...>::systemid = 0;
    }
}