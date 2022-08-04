#pragma once

/**
 * @file sparseset.h
 * @author Pigeon Codeur (pigeoncodeur@gmail.com)
 * @brief Definition of the sparse set container class
 * @version 0.1
 * @date 2022-08-02
 * 
 * @copyright Copyright (c) 2022
 */

#include <cstdint>
#include <vector>

#include "component.h"

namespace pg
{
    namespace ecs
    {
        // TODO set those typedef elsewhere and update this class accordingly
        typedef uint_fast64_t uint64;

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
            /**
             * @brief List representation of the component of the sparse set
             * 
             * @tparam Comp The type of the component
             * 
             * This helper class is used to iterate through the whole component list of the sparse set
             * It provides a basic [] interface as well as an iterator to support range based for loop
             */
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
                    // Public interface
                public:
                    /** 
                     * @brief Overload of the pre increment operator
                     * 
                     * @return The current iterator with the next indice
                     */
                    inline Iterator& operator++() { index++; return *this; }
                    
                    /**
                     * @brief Overload of the post increment operator
                     * 
                     * @return A new iterator with the next indice
                     */
                    inline Iterator operator++(int) { Iterator old = *this; index++; return old; }

                    /**
                     * @brief Overload of the equal operator
                     * 
                     * @param rhs Value to compare to
                     * 
                     * @return true if the value are equal
                     * @return false otherwise
                     */
                    inline bool operator==(const Iterator& rhs) const { return index == rhs.index; } 
                    
                    /**
                     * @brief Overload of the not equal operator
                     * 
                     * @param rhs Value to compare to
                     * 
                     * @return true if the value are not equal
                     * @return false otherwise
                     */
                    inline bool operator!=(const Iterator& rhs) const { return index != rhs.index; } 

                    /**
                     * @brief Overload of the * operator
                     * 
                     * @return Comp* A pointer to the component stored recasted into the actual component
                     */
                    inline Comp* operator*() const { return static_cast<Comp*>(componentList[index]);}

                    // Protected constructor
                protected:
                    /**
                     * @brief Construct a new Iterator object
                     * 
                     * @param pos The starting position in the component list array of the iterator
                     * @param componentList The component list to iterate over
                     * 
                     * This object can only be created from a SparseSet List inside of a SparseSet Object
                     */
                    Iterator(const size_t& pos, Component **componentList) : index(pos), componentList(componentList) {}

                    // Private variables
                private:
                    /** Index of the current position in the componentList */
                    size_t index = 1;
                    /** An array of component pointers */
                    Component **componentList;
                };

                // Public interface
            public:
                /**
                 * @brief Overload of the [] operator
                 * 
                 * @param index Position in the component list
                 * @return Comp* The pointer requested from the component list
                 * 
                 * This helper operator is used to provide access to a component inside of the component list.
                 * Be careful as the operator doesn't not check the bound of the list, this can throw an out of bound exception
                 * Use with nbElement of the sparse set to be in bound
                 */
                Comp* operator[](const size_t& index) { return static_cast<Comp*>(componentList[index]); }

                /**
                 * @brief Get the head iterator
                 * 
                 * @return constexpr Iterator An iterator at the head of the component list
                 */
                constexpr inline Iterator begin() const { return head; }

                /**
                 * @brief Get the tail iterator
                 * 
                 * @return constexpr Iterator An iterator at the tail of the component list
                 */
                constexpr inline Iterator end() const { return tail; }

                // Protected constructor
            protected:
                /**
                 * @brief Construct a new Sparse Set List object
                 * 
                 * @param size The current size of the component list
                 * @param componentList The component list to iterate over
                 * 
                 * This object can only be created from a SparseSet Object
                 */
                SparseSetList(const size_t& size, Component **componentList) : head(1, componentList), tail(size, componentList), componentList(componentList) {}

                // Private variables
            private:
                /** An iterator at the beginning of the list */
                const Iterator head;
                /** An iterator at the end of the list */
                const Iterator tail;

                /** The component list to iterate over */
                Component **componentList;      
            };
            
            // Publiv constructors
        public:
            /** Construct a new Sparse Set object */
            SparseSet();

            /** Destroy the Sparse Set object */
            ~SparseSet();
        
            // Public interface
        public:
            /**
             * @brief A check function to know if an entity as a component in this sparse set
             * 
             * @param id The id of the entity to check if it has a component
             * @return true if the entity id has a component in the list
             * @return false otherwise
             * 
             * This function uses one of the main properties of the sparse set, the reciprocity of the component id in the dense and sparse array
             * This operation is O(1) as it only need 2 indirections and 3 checks to know if an id is in the list and this is true whatever the size of the array
             */
            bool has(const uint64& id) const { return id < sparseCapacity && sparse[id] < denseCapacity && dense[sparse[id]] == id; };

            /**
             * @brief Get the id of the entity at a given index of the component list
             * 
             * @param index The index of the component
             * @return uint64 The id of the entity or 0 if the index is not inside of the list
             */
            uint64 at(const size_t& index) const
            {
                if(index >= size) 
                    return 0;
                
                return dense[index];
            }

            /**
             * @brief Get the id of the component of a given entity id if it is present in the component list
             * 
             * @param id The id of the entity to find in the component list
             * @return size_t The index of the component or 0 if the index is not inside of the list
             */
            size_t find(const uint64& id) const
            {
                if(has(id))
                    return sparse[id];

                return 0;
            }

            /** Add an entity and it's component inside of the list */
            Component* add(const uint64& entityId, Component* component);

            /** Remove a component by entity id */
            void remove(const uint64& id);

            /** Remove a component by component index*/
            void removeAt(const size_t& index);

            /** Clear the entire list */
            void clear();

            /**
             * @brief Get the current size of the list
             * 
             * @return constexpr size_t The current size of the list
             * 
             * The list start at index 1 to nbElement()
             * This function is used to ensure that the bound of the set are respected
             */
            constexpr size_t nbElements() const { return size; }

            /**
             * @brief Expose a view of the component list to another system
             * 
             * @tparam Comp The type of the component to cast the component stored in this list
             * @return SparseSetList<Comp> A view of the component list
             */
            template<typename Comp>
            SparseSetList<Comp> view()
            {
                return SparseSetList<Comp>(nbElements(), componentList);
            }

            // Private interface
        private:
            /** Internal helper function used to expend the dense and the component list */
            void addDenseCapacity();

            /** Internal helper function used to expend the sparse list */
            void addSparseCapacity(const uint64& entityId);

            // Private variables
        private:
            /** The current size of the sparse set */
            std::size_t size = 1;

            /** An interal array to hold the link componend id -> entity id */
            uint64* dense;

            /** The component list holding the data of all the component of this sparse set */
            Component** componentList;

            /** An interal array to hold the link entity id -> component id */
            size_t* sparse;

            /** The capacity of the dense array */
            size_t denseCapacity = 2;

            /** The capacity of the sparse array */
            size_t sparseCapacity = 2;
        };
    }    
}