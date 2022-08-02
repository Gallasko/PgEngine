#pragma once

#include <cstdint>
#include <vector>

#include "component.h"

#include <iostream>

namespace pg
{
    namespace ecs
    {
        typedef uint_fast64_t uint64;
        typedef uint_fast32_t entityId;

        // TODO

        /**
         * @brief A container object used to store components
         * 
         * This container is used to store components and the entity id they are associated with.
         * As entities ids as well as components id are unsigned value. The indice start at 1,
         * so that 0 indicates that the component doesn't exist or no entity was found.
         * 
         * This container as:
         * - O(1) time complexity for both inserting and removing components
         * - O(1) time complexity for lookup
         * - O(n) time complexity for clear (as it delete all the components in memory)
         * - O(n) time complexity for iteration over components
         * 
         * It also stores in memory the list of components of a given type
         * 
         * @todo Make a sparse set implementation that doesn't delete components on remove but instead reuse dead memory
         */
        class SparseSet
        {
        private:
            template<typename Comp>
            class SparseSetList
            {
            friend class SparseSet;
            public:
                /**
                 * @brief An Iterator for iterating over the elements of a sparse set
                 * 
                 * This iterator is a read only iterator that iterates over the elements ot the sparse set
                 * Using the operator-> you can obtain a pointer to the component 
                 */
                class Iterator
                {
                friend class SparseSet;
                friend class SparseSetList;
                public:
                    //Pre Increment
                    inline Iterator& operator++() { index++; return *this; }
                    
                    //Post Increment
                    inline Iterator operator++(int) { Iterator old = *this; index++; return old; }

                    inline bool operator==(const Iterator& rhs) const { return index == rhs.index; } 
                    inline bool operator!=(const Iterator& rhs) const { return index != rhs.index; } 

                    inline Comp* operator*() const { return static_cast<Comp*>((*componentList)[index]); }

                protected:
                    Iterator(const size_t& pos, std::vector<Component*> *componentList) : index(pos), componentList(componentList) {}

                private:
                    size_t index = 1;
                    std::vector<Component*> *componentList;
                };

            public:
                SparseSetList(const size_t& size, std::vector<Component*> *componentList) : head(1, componentList), tail(size, componentList) {}

                inline Iterator begin() const { return head; }
                inline Iterator end() const { return tail; }

            private:
                Iterator head;
                Iterator tail;                
            };
            
        public:
            bool has(const uint64& value) const { return value < sparse.capacity() && sparse[value] < dense.capacity() && dense[sparse[value]] == value; };

            uint64 at(const size_t& index) const
            {
                if(index >= size) 
                    return 0;
                
                return dense[index];
            }

            uint64 find(const uint64& value) const
            {
                if(has(value))
                    return sparse[value];

                return 0;
            }

            Component* add(const uint64& entityId, Component* component)
            {
                std::cout << size << std::endl;
                if(entityId < 1)
                {
                    // LOG_ERROR
                    return nullptr;
                }

                if(size >= dense.capacity())
                {
                    dense.reserve(size * 2);
                    componentList.reserve(size * 2);
                }

                if(entityId >= sparse.capacity())
                    sparse.reserve(entityId * 2);

                dense[size] = entityId;
                sparse[entityId] = size;

                componentList[size] = component;

                size++;

                return component;
            }

            /**
             * @brief Remove a component by entity id
             * 
             * @param id The id of the entity to remove the component from.
             */
            void remove(const uint64& id)
            {
                // Check if the id has a component
                if(size < 1 && !has(id))
                    return;

                // TODO make a sparse set implementation that doesn't delete components on remove but instead reuse dead memory
                // Delete the component
                delete componentList[sparse[id]];
                
                // Swap the last component in the place of the component to be removed
                componentList[sparse[id]] = componentList[sparse[size - 1]];

                // Update the index of the vector accordingly.
                dense[sparse[id]] = dense[sparse[size - 1]];
                sparse[id] = sparse[size - 1];

                // Decrease the size of the list
                size--;
            }

            /**
             * @brief Remove a component by component index
             * 
             * @param index The index of the component to remove.
             */
            void removeAt(const size_t& index)
            {
                if(size < 1 && index >= size) 
                    return;

                // TODO make a sparse set implementation that doesn't delete components on remove but instead reuse dead memory
                // Delete the component
                delete componentList[dense[index]];
                
                // Swap the last component in the place of the component to be removed
                componentList[dense[index]] = componentList[dense[size - 1]];

                // Put the last value where the removed value is, to keep a contiguous dense array
                // and update the sparse array accordingly.  
                sparse[dense[index]] = sparse[dense[size -1]];
                dense[index] = dense[size - 1];

                // Decrease the size of the list
                size--;

                // TODO Tell the management system that this entity (dense[index]) as lost this component
                // to update all the other "archtype" using it
                // Must implement a mutex for each component and entity for multithreaded use !
            }

            /**
             * @brief Clear the entire list
             * 
             * TODO make a sparse set implementation that doesn't delete components on remove but instead reuse dead memory
             */
            void clear()
            {
                size = 1;

                for(auto component : componentList)
                    delete component;
            }

            constexpr size_t nbElements() const { return size - 1; }

            template<typename Comp>
            SparseSetList<Comp> view() { return SparseSetList<Comp>(size, &componentList); }

        private:
            std::vector<uint64> dense;
            std::vector<uint64> sparse;
            std::vector<Component*> componentList;

            std::size_t size = 1;
        };
    }    
}