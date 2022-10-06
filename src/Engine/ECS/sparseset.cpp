#include "sparseset.h"

#include "logger.h"

#include <iostream>

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

            dense = new _unique_id[denseCapacity];

            // sparse.reserve(sparseCapacity);

            sparse = new size_t[sparseCapacity];
        }

        /**
         * @brief Destroy the Sparse Set object
         * 
         * Clear all the component in the component list
         * and destroy the internal arrays
         * 
         * @todo Check the cost of this virtual function
         */
        SparseSet::~SparseSet()
        {
            LOG_THIS_MEMBER(DOM);

            clear();

            std::lock_guard<std::mutex> lock(denseMutex);
            std::lock_guard<std::mutex> lock2(sparseMutex);

            delete[] dense;

            delete[] sparse;
        }

        /**
         * @brief Add an entity and it's component inside of the list
         * 
         * @param entity The entity to add to the list
         * @param component The data of the component to add to the list
         * 
         * @return Component* The pointer of the component added inside of the list
         * 
         * This function add the component to the list and link the id to the entity id
         * It also allocate new memory if one the list is too small
         * 
         * @todo Tell the management system that this entity (dense[index]) as created this component to update all the other "archtype" using it
         */
        size_t SparseSet::add(const _unique_id& id)
        {
            LOG_THIS_MEMBER(DOM);

            // All the entity should always be greater than 0 as 0 is the value of empty in the system
            if(id < 1)
            {
                LOG_ERROR(DOM, "Invalid entity id, must be greater than 0");
                return 0;
            }

            const size_t currentSize = size++;
            nbWorkingThread++;
            
            // If the size of the list is too small allocate more space
            if(denseCapacity <= currentSize)
            {
                LOG_INFO(DOM, "Dense array is too small (" + std::to_string(denseCapacity) + ") to fit the element: " + std::to_string(currentSize) + ", proceed to double the capacity");
                addDenseCapacity(currentSize);
            }

            // Link the entity id with the component id through the dense <-> sparse mechanism
            dense[currentSize] = id;
            denseNb++;

            if(resizingSparse != false || sparseCapacity <= id)
            {
                LOG_INFO(DOM, "Sparse array is too small (" + std::to_string(sparseCapacity) + ") to fit the element: " + std::to_string(id) + ", proceed to increase the capacity");
                addSparseCapacity(id, currentSize);
            }

            // Link the entity id with the component id through the dense <-> sparse mechanism
            sparse[id] = currentSize;
            sparseNb++;

            nbWorkingThread--;

            // Store the component inside of the list
            // componentList[size] = component;

            return currentSize;

            //return component;
        }

        /**
         * @brief Remove a component by entity id
         * 
         * @param entity The entity to remove the component from.
         * 
         * @todo Tell the management system that this entity (dense[index]) as lost this component to update all the other "archtype" using it
         * @todo Must implement a mutex for each component and entity for multithreaded use !
         * @todo check that it still work in a multithreaded environment
         */
        size_t SparseSet::remove(const _unique_id& id)
        {
            LOG_THIS_MEMBER(DOM);

            const size_t currentSize = size--;

            // Check if the id has a component
            if(currentSize < 1 && !has(id))
                return 0;
            
            std::lock_guard<std::mutex> lock(denseMutex);
            std::lock_guard<std::mutex> lock2(sparseMutex);

            const auto index = sparse[id];

            // Update the index of the vector accordingly.
            dense[index] = dense[sparse[currentSize - 1]];
            denseNb--;
            sparse[id] = sparse[currentSize - 1];
            sparseNb--;
            return index;
        }


        // /**
        //  * @brief Remove a component by component index
        //  * 
        //  * @param index The index of the component to remove.
        //  * 
        //  * @todo Tell the management system that this entity (dense[index]) as lost this component to update all the other "archtype" using it
        //  * @todo Must implement a mutex for each component and entity for multithreaded use !
        //  */
        // void SparseSet::removeAt(const size_t& index)
        // {
        //     LOG_THIS_MEMBER(DOM);

        //     if(size < 1 && index >= size) 
        //         return;

        //     // TODO make a sparse set implementation that doesn't delete components on remove but instead reuse dead memory
        //     // Delete the component
        //     delete componentList[dense[index]];
            
        //     // Swap the last component in the place of the component to be removed
        //     componentList[dense[index]] = componentList[dense[size - 1]];

        //     // Put the last value where the removed value is, to keep a contiguous dense array
        //     // and update the sparse array accordingly.  
        //     sparse[dense[index]] = sparse[dense[size -1]];
        //     dense[index] = dense[size - 1];

        //     // Decrease the size of the list
        //     size--;
        // }

        /**
         * @brief Clear the entire list
         * 
         * @todo make a sparse set implementation that doesn't delete components on remove but instead reuse dead memory
         */
        void SparseSet::clear()
        {
            LOG_THIS_MEMBER(DOM);

            size = 1;
        }

        /**
         * @brief Internal helper function used to expend the dense and the component list
         * 
         * This helper function double both the size of the dense and the component list to store up to size_t amount of data 
         */
        void SparseSet::addDenseCapacity(const size_t& size)
        {
            LOG_THIS_MEMBER(DOM);

            if(denseCapacity > size)
                return;
            
            if(size > SIZE_MAX)
            {
                LOG_ERROR(DOM, "Entity id is too large to fit into sparse set");
                return;
            }

            while(denseNb < size);

            std::lock_guard<std::mutex> lock(denseMutex);

            if(denseCapacity > size)
                return;

            size_t targetCapacity = denseCapacity;

            // This small loop make it so denseCapacity stays as a multiple of 2
            while(targetCapacity <= size)
            {
                targetCapacity *= 2;
            }

            // Create the doubled size containers
            _unique_id* tempDense = new _unique_id[targetCapacity];
            
            // Copy the current data inside of the newly created containers
            memcpy(tempDense, dense, denseCapacity * sizeof(_unique_id));
            
            // Delete old data to not leak memory
            delete[] dense;
            
            // Set the new containers as the list container
            dense = tempDense;

            // Update the capacity of the list
            denseCapacity = targetCapacity;
        }

        /**
         * @brief Internal helper function used to expend the sparse list
         * 
         * @param[in] id The id to fit inside of the sparse array
         * 
         * This helper function double the size of the sparse to store up to size_t amount of data 
         */
        void SparseSet::addSparseCapacity(const _unique_id& id, const size_t& size)
        {
            LOG_THIS_MEMBER(DOM);

            resizingSparse = true;
            inResize++;

            if(resizingSparse == false && sparseCapacity > id)
                return;

            // If the entity is bigger than SIZE_MAX then it can't be expressed as a size_t and so it can't be stored in the array.
            if(resizingSparse == false && id > SIZE_MAX)
            {
                LOG_ERROR(DOM, "Entity id is too large to fit into sparse set");
                return;
            }

            // waitingSparseNb++;

            while(resizingSparse == true && inResize != nbWorkingThread.load()) std::cout << resizingSparse.load() << " " << sparseNb.load() << " " << inResize.load() << " " << nbWorkingThread.load() << " " << sparseCapacity << std::endl;

            // waitingSparseNb--;

            std::lock_guard<std::mutex> lock(sparseMutex);

            if(resizingSparse != true || inResize != nbWorkingThread.load())
            {
                inResize--;
                return;
            }

            if(sparseCapacity > id)
            {
                inResize--;
                return;
            }

            size_t targetCapacity = sparseCapacity;

            // This small loop make it so sparseCapacity stays as a multiple of 2
            while(targetCapacity <= id)
            {
                targetCapacity *= 2;
            }

            // sparse.reserve(targetCapacity);

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

            inResize--;
            resizingSparse = false;
        }
    }
}