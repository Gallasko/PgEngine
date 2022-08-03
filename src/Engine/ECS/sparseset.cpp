#include "sparseset.h"

#include "logger.h"

namespace pg
{
    namespace ecs
    {
        namespace
        {
            const char * DOM = "Sparse Set";
        }

        /**
         * @brief Construct a new Sparse Set object
         * 
         * Initialize all the internal arrays
         * Component list should always strictly match the dense array
         */
        SparseSet::SparseSet()
        {
            LOG_THIS_MEMBER(DOM);

            dense = new uint64[denseCapacity];
            componentList = new Component*[denseCapacity];

            sparse = new size_t[sparseCapacity];
        }

        /**
         * @brief Destroy the Sparse Set object
         * 
         * Clear all the component in the component list
         * and destroy the internal arrays
         */
        SparseSet::~SparseSet()
        {
            LOG_THIS_MEMBER(DOM);

            clear();

            delete[] dense;
            delete[] componentList;

            delete[] sparse;
        }

        /**
         * @brief Add an entity and it's component inside of the list
         * 
         * @param entityId The id of the entity to add to the list
         * @param component The data of the component to add to the list
         * 
         * @return Component* The pointer of the component added inside of the list
         * 
         * This function add the component to the list and link the id to the entity id
         * It also allocate new memory if one the list is too small
         * 
         * @todo Tell the management system that this entity (dense[index]) as created this component to update all the other "archtype" using it
         */
        Component* SparseSet::add(const uint64& entityId, Component* component)
        {
            LOG_THIS_MEMBER(DOM);

            // All the entity should always be greater than 0 as 0 is the value of empty in the system
            if(entityId < 1)
            {
                LOG_ERROR(DOM, "Invalid entity id, must be greater than 0");
                return nullptr;
            }

            // If the size of the list is too small allocate more space
            if(size >= denseCapacity)
            {
                LOG_INFO(DOM, "Dense array is too small (" + std::to_string(denseCapacity) + ") to fit the element: " + std::to_string(size) + ", proceed to double the capacity");
                addDenseCapacity();
            }

            if(entityId >= sparseCapacity)
            {
                LOG_INFO(DOM, "Dense array is too small (" + std::to_string(denseCapacity) + ") to fit the element: " + std::to_string(size) + ", proceed to increase the capacity");
                addSparseCapacity(entityId);
            }

            // Link the entity id with the component id through the dense <-> sparse mechanism
            dense[size] = entityId;
            sparse[entityId] = size;

            // Store the component inside of the list
            componentList[size] = component;

            // Increase the side of the list
            size++;

            return component;
        }

        /**
         * @brief Remove a component by entity id
         * 
         * @param id The id of the entity to remove the component from.
         * 
         * @todo Tell the management system that this entity (dense[index]) as lost this component to update all the other "archtype" using it
         * @todo Must implement a mutex for each component and entity for multithreaded use !
         */
        void SparseSet::remove(const uint64& id)
        {
            LOG_THIS_MEMBER(DOM);

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
         * 
         * @todo Tell the management system that this entity (dense[index]) as lost this component to update all the other "archtype" using it
         * @todo Must implement a mutex for each component and entity for multithreaded use !
         */
        void SparseSet::removeAt(const size_t& index)
        {
            LOG_THIS_MEMBER(DOM);

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
        }

        /**
         * @brief Clear the entire list
         * 
         * @todo make a sparse set implementation that doesn't delete components on remove but instead reuse dead memory
         */
        void SparseSet::clear()
        {
            LOG_THIS_MEMBER(DOM);

            for(size_t i = 1; i < size; i++)
                delete componentList[i];

            size = 1;
        }

        /**
         * @brief Internal helper function used to expend the dense and the component list
         * 
         * This helper function double both the size of the dense and the component list to store up to size_t amount of data 
         */
        void SparseSet::addDenseCapacity()
        {
            LOG_THIS_MEMBER(DOM);

            // Create the doubled size containers
            uint64* tempDense = new uint64[denseCapacity * 2];
            Component** tempComponentList = new Component*[denseCapacity * 2];

            // Copy the current data inside of the newly created containers
            memcpy(tempDense, dense, denseCapacity * sizeof(uint64));
            memcpy(tempComponentList, componentList, denseCapacity * sizeof(Component*));

            // Delete old data to not leak memory
            delete[] dense;
            delete[] componentList;

            // Set the new containers as the list container
            dense = tempDense;
            componentList = tempComponentList;

            // Update the capacity of the list
            denseCapacity *= 2;
        }

        /**
         * @brief Internal helper function used to expend the sparse list
         * 
         * @param[in] entityId The id to fit inside of the sparse array
         * 
         * This helper function double the size of the sparse to store up to size_t amount of data 
         */
        void SparseSet::addSparseCapacity(const uint64& entityId)
        {
            LOG_THIS_MEMBER(DOM);

            // If the entity is bigger than SIZE_MAX then it can't be expressed as a size_t and so it can't be stored in the array.
            if(entityId > SIZE_MAX)
            {
                LOG_ERROR(DOM, "Entity id is too large to fit into sparse set");
                return;
            }

            size_t targetCapacity = sparseCapacity;

            // This small loop make it so sparseCapacity stays as a multiple of 2
            while(targetCapacity < entityId)
            {
                targetCapacity *= 2;
            }

            // Create a bigger size container
            size_t* tempSparse = new size_t[targetCapacity];

            // Copy the current data inside of the newly created container
            memcpy(tempSparse, sparse, sparseCapacity * sizeof(size_t));

            // Delete old data to not leak memory
            delete[] sparse;

            // Set the new container as the list container
            sparse = tempSparse;

            // Update the capacity of the list
            sparseCapacity = targetCapacity;
        }
    }
}