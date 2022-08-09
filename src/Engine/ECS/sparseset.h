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

#include "entity.h"
#include "component.h"

namespace pg
{
    namespace ecs
    {
        // TODO make doc
        class SparseSet
        {
        private:
            // Todo make doc
            class SparseSetList
            {
            friend class SparseSet;
            public:
                // TODO make doc
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
                     * @return _unique_id A unique identifier in the list
                     */
                    inline _unique_id operator*() const { return dense[index]; }

                    // Protected constructor
                protected:
                    Iterator(const size_t& pos, _unique_id *dense) : index(pos), dense(dense) {}

                    // Private variables
                private:
                    size_t index = 1;

                    _unique_id *dense;
                };

                // Public interface
            public:
                // Todo make doc
                _unique_id operator[](const size_t& index) { return dense[index]; }

                /**
                 * @brief Get the head iterator
                 * 
                 * @return constexpr Iterator An iterator at the head of the dense list
                 */
                inline Iterator begin() const { return head; }

                /**
                 * @brief Get the tail iterator
                 * 
                 * @return constexpr Iterator An iterator at the tail of the dense list
                 */
                inline Iterator end() const { return tail; }

                // Public constructor
            public:
                /**
                 * @brief Construct a copy of a Sparse Set List object
                 * 
                 * @param other The Sparse Set List to copy
                 */
                SparseSetList(const SparseSetList& other) : head(other.head), tail(other.head), dense(other.dense) {}

                // Protected constructor
            protected:
                // Todo make doc
                SparseSetList(const size_t& size, _unique_id *dense) : head(1, dense), tail(size, dense), dense(dense) {}

                // Private variables
            private:
                /** An iterator at the beginning of the list */
                const Iterator head;
                /** An iterator at the end of the list */
                const Iterator tail;

                /** The dense list to iterate over */
                _unique_id *dense;      
            };
            
            // Public constructors
        public:
            /** Construct a new Sparse Set object */
            SparseSet();

            /** Destroy the Sparse Set object */
            virtual ~SparseSet();
        
            // Public interface
        public:
            /**
             * @brief A check function to know if an id is in this sparse set
             * 
             * @param id The id to check if it is in the sparse set
             * @return true if the id is in the set
             * @return false otherwise
             * 
             * This function uses one of the main properties of the sparse set, the reciprocity of the id in the dense and sparse array
             * This operation is O(1) as it only need 2 indirections and 3 checks to know if an id is in the list and this is true whatever the size of the array
             */
            inline bool has(const _unique_id& id) const { return id < sparseCapacity && sparse[id] < denseCapacity && dense[sparse[id]] == id; };

            /**
             * @brief Get the id at a given index of the set
             * 
             * @param index The index to look for in the set
             * @return _unique_id The id or 0 if the index is not inside of the set
             */
            inline _unique_id at(const size_t& index) const
            {
                if(index >= size) 
                    return 0;
                
                return dense[index];
            }

            /**
             * @brief Get the index of a given id if it is present in the set
             * 
             * @param id The id to store in the set
             * @return size_t The index of the id or 0 if the index is not inside of the set
             */
            inline size_t find(const _unique_id& id) const
            {
                if(has(id))
                    return sparse[id];

                return 0;
            }

            /** Add an entity and it's component inside of the list */
            size_t add(const _unique_id& id);

            /** Remove a component by entity id */
            size_t remove(const _unique_id& id);

            // /** Remove a component by component index*/
            // void removeAt(const size_t& index);

            /** Clear the entire list */
            inline void clear();

            /**
             * @brief Get the current size of the list
             * 
             * @return constexpr size_t The current size of the list
             * 
             * The list start at index 1 to nbElement()
             * This function is used to ensure that the bound of the set are respected
             */
            inline constexpr size_t nbElements() const { return size; }

            inline SparseSetList view() const
            {
                return SparseSetList(nbElements(), dense);
            }

            // Protected variables
        protected:
            /** The current size of the sparse set */
            size_t size = 1;

            // Private interface
        private:
            /** Internal helper function used to expend the dense and the component list */
            void addDenseCapacity();

            /** Internal helper function used to expend the sparse list */
            void addSparseCapacity(const _unique_id& id);

            // Private variables
        private:
            /** An interal array to hold the link componend id -> entity id */
            _unique_id* dense;

            /** An interal array to hold the link entity id -> component id */
            size_t* sparse;

            /** The capacity of the dense array */
            size_t denseCapacity = 2;

            /** The capacity of the sparse array */
            size_t sparseCapacity = 2;
        };

        // 
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
         * @warning Never delete a ComponentSet through a SparseSet pointer
         * 
         * @todo Make a sparse set implementation that doesn't delete components on remove but instead reuse dead memory
         */
        template <typename Comp>
        class ComponentSet : public SparseSet
        {
        public:
            /**
             * @brief List representation of the component of the component set
             * 
             * This helper class is used to iterate through the whole component list of the component set
             * It provides a basic [] interface as well as an iterator to support range based for loop
             */
            class ComponentSetList
            {
            friend class ComponentSet;
            public:
                /**
                 * @brief An Iterator for iterating over the elements of a component set
                 * 
                 * This iterator is a read only iterator that iterates over the elements ot the component set
                 * Using the operator-> you can obtain a pointer to the component 
                 */
                class Iterator
                {
                friend class ComponentSet;
                friend class ComponentSetList;
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
                    inline Comp* operator*() const { return componentList[index];}

                    // Protected constructor
                protected:
                    /**
                     * @brief Construct a new Iterator object
                     * 
                     * @param pos The starting position in the component list array of the iterator
                     * @param componentList The component list to iterate over
                     * 
                     * This object can only be created from a ComponentSet List inside of a ComponentSet Object
                     */
                    Iterator(const size_t& pos, Comp **componentList) : index(pos), componentList(componentList) {}

                    // Private variables
                private:
                    /** Index of the current position in the componentList */
                    size_t index = 1;
                    /** An array of component pointers */
                    Comp **componentList;
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
                inline Iterator begin() const { return head; }

                /**
                 * @brief Get the tail iterator
                 * 
                 * @return constexpr Iterator An iterator at the tail of the component list
                 */
                inline Iterator end() const { return tail; }

                // Public constructor
            public:
                /**
                 * @brief Construct a copy of a Sparse Set List object
                 * 
                 * @param other The Sparse Set List to copy
                 */
                ComponentSetList(const ComponentSetList& other) : head(other.head), tail(other.head), componentList(other.componentList) {}

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
                ComponentSetList(const size_t& size, Comp **componentList) : head(1, componentList), tail(size, componentList), componentList(componentList) {}

                // Private variables
            private:
                /** An iterator at the beginning of the list */
                Iterator head;
                /** An iterator at the end of the list */
                Iterator tail;

                /** The component list to iterate over */
                Comp **componentList;      
            };

        public:
            ComponentSet() : SparseSet()
            {
                componentList = new Comp*[componentCapacity];
            };

            ~ComponentSet()
            {
                for(size_t i = 1; i < maxAchievedComponents; i++)
                    delete componentList[i];

                delete[] componentList;
            }

            Comp* addComponent(const Entity& entity, Comp* component)
            {
                const auto index = add(entity.id);

                if(index == 0)
                    return nullptr;

                if(index >= componentCapacity)
                {
                    Comp** tempComponentList = new Comp*[componentCapacity * 2];
                    memcpy(tempComponentList, componentList, componentCapacity * sizeof(Comp*));
                    delete[] componentList;
                    componentList = tempComponentList;
                    componentCapacity *= 2;
                }

                if(index > maxAchievedComponents)
                    maxAchievedComponents = index;
                else if(index < maxAchievedComponents)
                    delete componentList[index];

                lastEntityIndex = index;
                componentList[index] = component;

                return component;
            }

            // TODO make a sparse set implementation that doesn't delete components on remove but instead reuse dead memory
            void removeComponent(const Entity& entity)
            {
                const auto index = remove(entity.id);

                if(index == 0)
                    return;

                // Swap the last component in the place of the component to be removed
                componentList[index] = componentList[lastEntityIndex];
            }

            /**
             * @brief Expose a view of the component list to a system
             * 
             * @tparam Comp The type of the component to cast the component stored in this list
             * @return ComponentSetList A view of the component list
             */
            inline ComponentSetList viewComponents() const
            {
                return ComponentSetList(size, componentList);
            }

        private:
            /** The component list holding the data of all the component of this sparse set */
            Comp** componentList;

            size_t maxAchievedComponents = 1;

            size_t componentCapacity = 2;

            size_t lastEntityIndex = 0;
        };
    }    
}