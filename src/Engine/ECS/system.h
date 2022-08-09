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

                this->registry = registry;

                registerComponents(this, registry, tag<Comps>{}...);
            }

            template <typename Type, typename... Args>
            inline Type* createComponent(Entity& entity, const Args&... args)
            {
                LOG_THIS_MEMBER("System");

                return this->createRefferedComponent<Type>(entity, args...);
            }

            template <typename Type, typename... Args>
            inline Type* createOwnedComponent(Entity& entity, const Args&... args)
            {
                LOG_THIS_MEMBER("System");

                return this->Own<Type>::internalCreateComponent(entity, args...);
            }

            template <typename Type, typename... Args>
            inline Type* createRefferedComponent(Entity& entity, const Args&... args)
            {
                LOG_THIS_MEMBER("System");

                return this->Ref<Type>::internalCreateComponent(entity, args...);
            }

            template <typename Type>
            inline void removeComponent(Entity& entity)
            {
                LOG_THIS_MEMBER("System");

                this->removeRefferedComponent(entity);
            }

            template <typename Type>
            void removeRefferedComponent(Entity& entity)
            {
                LOG_THIS_MEMBER("System");

                this->Ref<Type>::internalRemoveComponent(entity);
            }

            template <typename Type>
            inline void removeOwnedComponent(Entity& entity)
            {
                LOG_THIS_MEMBER("System");

                this->Own<Type>::internalRemoveComponent(entity);
            }

            template <typename Type>
            inline const typename ComponentSet<Type>::ComponentSetList& view() const
            {
                LOG_THIS_MEMBER("System");

                return this->Ref<Type>::view();
            }

            template <typename Type, typename... Types>
            Group<Type, Types...> group() const
            {
                LOG_THIS_MEMBER("System");

                if(registry == nullptr)
                {
                    LOG_ERROR("System", "No registry specified, can't create a group");
                    return Group<Type, Types...>();
                }

                if(Group<Type, Types...>::groupId != 0)
                    return *(registry->retrieve<Type, Types...>());
                else
                {
                    auto group = new Group<Type, Types...>(generateId());
                    
                    group->setRegistry(registry);
                }

            }

            ComponentRegistry *registry = nullptr;

            static _unique_id systemid;
        };

        template <typename... Comps>
        _unique_id System<Comps...>::systemid = 0;
    }
}