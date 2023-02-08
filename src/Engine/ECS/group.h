/**
 * @file group.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-09-02
 * 
 * @copyright Copyright (c) 2022
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 */
#pragma once

#include "uniqueid.h"
#include "sparseset.h"

#include "componentregistry.h"

#include "logger.h"

#include <algorithm>

// Todo to replace with the thread pool manager
#include <functional>

#include <iostream>

#include "Memory/threadpool.h"
#include "Memory/parallelfor.h"

namespace pg
{
    // Type forwarding
    class ComponentRegistry;

    template <typename Type>
    struct Getter
    {
        Getter() : value(nullptr) {}
        Getter(Type* value) : value(value) { LOG_THIS_MEMBER("Ecs Group"); }

        Type* get() const { LOG_THIS_MEMBER("Ecs Group"); return value; }
        void set(Type* value) { LOG_THIS_MEMBER("Ecs Group"); this->value = value; }

        Type* value; 
    };

    template <typename... Types>
    struct GroupElement : public Getter<Types>...
    {
        GroupElement(const _unique_id& entityId) : Getter<Types>()..., entityId(entityId) {}
        GroupElement(const _unique_id& entityId, Types*... values) : Getter<Types>(values)..., entityId(entityId) { LOG_THIS_MEMBER("Ecs Group"); }

        template <typename Type>
        Type* get() const { LOG_THIS_MEMBER("Ecs Group"); return static_cast<const Getter<Type>*>(this)->get(); }

        template <typename Type>
        void set(Type *value) { LOG_THIS_MEMBER("Ecs Group"); static_cast<Getter<Type>*>(this)->set(value); }

        const _unique_id entityId;
        bool toBeDeleted = false;
    };

    template <typename Type, typename... Types>
    struct SetHolder
    {
        SetHolder(const SparseSet* set, void(*f)(const SparseSet*, GroupElement<Type, Types...>&, size_t)) : set(set), setElement(f) {}

        const SparseSet* set;
        void (*setElement) (const SparseSet*, GroupElement<Type, Types...>&, size_t);
    };

    template <typename Set, typename Type, typename... Types>
    void getFromSet(const SparseSet* set, GroupElement<Type, Types...>& element, size_t id)
    {
        const auto& pos = static_cast<const Set*>(set)->find(id);
        if(pos != 0)
            element.set((*static_cast<const Set*>(set))[pos]);
        else
            element.toBeDeleted = true;
    }

    template <typename Type, typename... Types>
    struct Group
    {
        Group(_unique_id id) : id(id) { LOG_THIS_MEMBER("Ecs Group"); }
        virtual ~Group() { LOG_THIS_MEMBER("Ecs Group"); }

        void setRegistry(ComponentRegistry* registry)
        {
            LOG_THIS_MEMBER("Ecs Group");

            this->registry = registry;
            registry->storeGroup<Type, Types...>(this);
        }

        void addOnGroup(void(*callback)(Entity*))
        {
            onAddGroup.push_back(callback);

            for(const auto& element : elements.viewComponents())
            {
                callback(element->entityId);
            }
        }

        void process()
        {
            LOG_THIS_MEMBER("Ecs Group");

            constexpr size_t nbOfSets = sizeof...(Types) + 1;

            SetHolder<Type, Types...> *setList[nbOfSets];

            populateList(setList, 0, registry->retrieve<Type>()->components, registry->retrieve<Types>()->components...);

            size_t smallestSetIndex = 0;

            for(size_t i = 0; i < nbOfSets; ++i)
            {
                if(setList[i]->set->nbElements() < setList[smallestSetIndex]->set->nbElements())
                    smallestSetIndex = i;
            }

            const SparseSet* smallestSet = setList[smallestSetIndex]->set;

            setList[smallestSetIndex] = setList[nbOfSets - 1];

            // const auto& elements = {registry->retrieve<Type>()->components, registry->retrieve<Types>()->components...};

            // const SparseSet& set = smallestSet(registry->retrieve<Type>()->components, registry->retrieve<Types>()->components...);

            LOG_INFO("Ecs Group", "Smallest set has: " + std::to_string(smallestSet->nbElements()) + " elements");

            // Todo add reserve and multiple emplace back in the component/sparse set
            // elements.reserve(smallestSet->nbElements()); // May need a -1

            // Todo check Branch: Parallel-Ecs to create a parallal implementation of grouping
            for(const auto& id : smallestSet->view())
            {
                GroupElement<Type, Types...> element(id);

                for(size_t j = 0; j < nbOfSets - 1; j++)
                {
                    setList[j]->setElement(setList[j]->set, element, id);    
                }

                if(not element.toBeDeleted)
                    elements.addComponent(id, element);
            }

            for(size_t i = 0; i < nbOfSets; i++)
                delete setList[i];

            // Add support for thread pools by passing a pool in this function and add the task inside of this pool
            // checkEntityInGroup<Type, Types...>(this->registry->getThreadPool(), elements);

            // const auto& it = elements.viewComponents();

            // Remove all elements that miss at least one component from the group
            // Todo Do not delete the component but make the iterator skip element to be deleted !
            // for(size_t i = 1; i < elements.nbElements(); i++)
            // {
                // const auto& element = elements[i];
                // if(element->toBeDeleted)
                    // elements.removeComponent(element->entityId);
            // }
            //std::remove_if(it.begin(), it.end(), [](const GroupElement<Type, Types...>& element) { return element.toBeDeleted; });
        
            // Todo sort the group
        }

        template <typename Set>
        inline void populateList(SetHolder<Type, Types...> **list, size_t index, const Set& setN)
        {
            LOG_THIS_MEMBER("Ecs Group");

            list[index] = new SetHolder<Type, Types...> ( &setN, &getFromSet<Set, Type, Types...> );
        }

        template <typename Set, typename... Sets>
        inline void populateList(SetHolder<Type, Types...> **list, size_t index, const Set& setN, const Sets&... sets)
        {
            LOG_THIS_MEMBER("Ecs Group");

            list[index] = new SetHolder<Type, Types...> ( &setN, &getFromSet<Set, Type, Types...> );

            populateList(list, index + 1, sets...);
        }

        inline const SparseSet& smallestSet(const SparseSet& set1, const SparseSet& set2) const
        {
            LOG_THIS_MEMBER("Ecs Group");

            return set1.nbElements() < set2.nbElements() ? set1 : set2;
        }

        template <typename Set, typename... Sets>
        inline const SparseSet& smallestSet(const SparseSet& set1, const SparseSet& set2, const Set& setN, const Sets&... sets) const
        {
            LOG_THIS_MEMBER("Ecs Group");

            return set1.nbElements() < set2.nbElements() ? smallestSet(set1, setN, sets...) : smallestSet(set2, setN, sets...);
        }

        // End case of the recursion
        template <typename Value>
        inline void checkEntityInGroup(ThreadPool* pool, GroupSet<GroupElement<Type, Types...>>& elements)
        {
            LOG_THIS_MEMBER("Ecs Group");

            const auto& components = registry->retrieve<Value>()->components;

            // Create Task
            auto result = pool->enqueue( [&elements, &components]() -> void
            {
                for (const auto& element : elements.viewComponents())
                {
                    const auto& pos = components.find(element->entityId);
                    if(pos != 0)
                        element->set(components[pos]);
                    else
                        element->toBeDeleted = true;
                }  
            });

            // TODO
            // Send task to the thread pool ( pool->addTask(task); )
            //task();

            result.get();

            // Todo join the task here !
        }

        template <typename Value, typename Other, typename... Values>
        inline void checkEntityInGroup(ThreadPool* pool, GroupSet<GroupElement<Type, Types...>>& elements)
        {
            LOG_THIS_MEMBER("Ecs Group");

            const auto& components = registry->retrieve<Value>()->components;

            // Create Task
            auto result = pool->enqueue( [&elements, &components]() -> void
            {
                for (const auto& element : elements.viewComponents())
                {
                    const auto& pos = components.find(element->entityId);
                    if(pos != 0)
                        element->set(components[pos]);
                    else
                        element->toBeDeleted = true;
                }  
            });

            // TODO
            // Send task to the thread pool ( pool->addTask(task); )
            // task();

            // Recursive call to add all the different components to the group
            checkEntityInGroup<Other, Values...>(pool, elements);

            result.get();

            // Todo join the task here !
        }

        _unique_id id;
        ComponentRegistry* registry;
        GroupSet<GroupElement<Type, Types...>> elements;

        std::vector<void(Entity*)> onAddGroup;
        std::vector<void(Entity*)> onDelGroup;
    };
}