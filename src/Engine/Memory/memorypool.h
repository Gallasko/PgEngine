#pragma once

/**
 * @file memorypool.h
 * @author Pigeon Codeur
 * @brief Definition of the memory pool 
 * @version 0.1
 * @date 2022-05-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <type_traits>
#include <vector>
#include <memory>
#include <mutex>
#include <cmath>
#include <atomic>

#include "logger.h"
namespace pg
{
    static constexpr unsigned int tab64[64] = {
        63,  0, 58,  1, 59, 47, 53,  2,
        60, 39, 48, 27, 54, 33, 42,  3,
        61, 51, 37, 40, 49, 18, 28, 20,
        55, 30, 34, 11, 43, 14, 22,  4,
        62, 57, 46, 52, 38, 26, 32, 41,
        50, 36, 17, 19, 29, 10, 13, 21,
        56, 45, 25, 31, 35, 16,  9, 12,
        44, 24, 15,  8, 23,  7,  6,  5};

    static int log2_64 (size_t value)
    {
        value |= value >> 1;
        value |= value >> 2;
        value |= value >> 4;
        value |= value >> 8;
        value |= value >> 16;
        value |= value >> 32;

        return tab64[((size_t)((value - (value >> 1))*0x07EDD5E59A4E28C2)) >> 58];
    }

    /**
     * @tparam T Type of the underlying object
     * 
     * Union representing a chunk of memory mananaged by the pool.
     * It can be either a single object or a pointer to the next free space in the pool
     */
    template<typename T>
    union Chunk
    {
        /** Storage for a single object */
        typename std::aligned_storage<sizeof(T), alignof(T)>::type element;

        /** Pointer to the next free space in the pool */
        Chunk *next;
    };

    /**
     * @brief A helper struct to create a block of memory
     * 
     * @tparam T Type of the object to be created
     * @tparam N 
     */
    template<typename T>
    struct Block
    {
        /**
         * @brief Construct a new Block object
         * 
         * @param size Number of object to allocate at once
         * 
         * Create N free object and then passed the ownership of those element to the pool
         */
        Block(const size_t& size)
        {
            LOG_THIS_MEMBER("Memory Pool");

            // Allocate at once enough space for N objects
            chunks = new Chunk<T>[size];
/*
            // Construct the free list of objects
            for(unsigned int i = 0; i < size - 1; i++)
            {
                chunks[i].next = &chunks[i + 1];
            }

            // End the free list
            chunks[size - 1].next = nullptr;
*/
        }

        /** The first free space of the newly created block */
        Chunk<T>* chunks;
    };

    /**
     * @brief An implementation of an allocator pool
     * 
     * @tparam T Type of the object to be created
     * @tparam N if N > 1, Number of object to be created at once when running out of empty element else the pool expand exponentially (default at 1)
     */
    template<typename T, size_t N = 1>
    class AllocatorPool
    {
    public:
        /**
         * @brief Destroy the Allocator Pool object
         * 
         * Delete all the object given back to the pool.
         * If the user forget to release memory, memory leaks can occur !
         * 
         * @see release
         */
        ~AllocatorPool()
        {
            LOG_THIS_MEMBER("Memory Pool");

            for(Chunk<T>* chunk : chunkList)
                delete chunk;
        }

        template<bool internalCall = false>
        void reserve(size_t reserveSize)
        {
            LOG_THIS_MEMBER("Memory Pool");

            LOG_MILE("Memory Pool", "Trying to reserve: " + std::to_string(reserveSize) +  ", currentPoolSize = " +
                std::to_string(size) + " " +
                std::to_string(currentSize) + " " +
                std::to_string(nbElements));

            if(reserveSize < size) return;

            if(internalCall)
                while(currentSize != size);
            else
                while(currentSize != nbElements.load());

            std::lock_guard<std::recursive_mutex> lock(mutex);

            LOG_INFO("Memory Pool", "Reserving: " + std::to_string(reserveSize) +  ", currentPoolSize = " +
                std::to_string(size) + " " +
                std::to_string(currentSize) + " " +
                std::to_string(nbElements));

            while (reserveSize >= size)
            {
                const size_t blockSize = N >= 2 ? N : size == 0 ? 1 : size + 1;

                LOG_MILE("Memory Pool", "Current size: " + std::to_string(size) + ", target: " +std::to_string(reserveSize) + ", blockSize: " + std::to_string(blockSize));

                auto newBlock = Block<T>(blockSize);
                freeList = newBlock.chunks;

                chunkList.push_back(newBlock.chunks);

                size += blockSize;
            }
        }

        /**
         * @brief Function used to allocate a new T object
         * 
         * @tparam Args Type of the arguments to be passed to create an object
         * @param args Argument to create a new T object
         * @return T* A pointer to the new T object created
         * 
         * To be used instead of the default new operator to construct an object using the pool
         * To destroy this object use the release method of the pool
         * 
         * @see release
         */
        template<typename... Args>
        T* allocate(Args&&... args)
        {
            LOG_THIS_MEMBER("Memory Pool");

            const size_t index = nbElements++;

            if(index >= size) reserve<true>(index);

            const unsigned int n = log2_64(index + 1);
            const size_t containerSize = N >= 2 ? N : n == 0 ? 0 : 1 << n;

            const size_t listPos = N >= 2 ? index / containerSize : n;
            const size_t vectorPos = N >= 2 ? index % containerSize : n == 0 ? 0 : index + 1 - containerSize;

            LOG_MILE("Memory Pool", Strfy() <<
                "Allocating new element: " << index <<
                " in chunk: "  << listPos <<
                " at pos: " << vectorPos <<
                " with current pool size: " << size);

            Chunk<T>* chunk = &chunkList[listPos][vectorPos];

            currentSize++;

            ::new(&(chunk->element)) T(std::forward<Args>(args)...);

            return reinterpret_cast<T*>(chunk);
        } 

        /**
         * @brief Function used to release the memory of a T object create using the pool
         * 
         * @param pointer A pointer to a T object
         * 
         * Use this function to release memory of a T object created using the allocate function
         * 
         * @see allocate
         */
        void release(T* pointer)
        {
            LOG_THIS_MEMBER("Memory Pool");

            if(pointer != nullptr)
            {
                pointer->~T();

                // {
                //     std::lock_guard<std::recursive_mutex> lock(mutex);

                //     reinterpret_cast<Chunk<T>*>(pointer)->next = freeList;
                //     freeList = reinterpret_cast<Chunk<T>*>(pointer);
                // }
            }
        }

    private:
        size_t size = 0;

        std::atomic<size_t> nbElements{0};
        std::atomic<size_t> currentSize{0};

        // Todo implement back the freelist and used it only on deleted free space
        /** Pointer to the next free object in the pool */
        Chunk<T>* freeList = nullptr;

        /** Chunk Lists used in the pool (used to free the memory) */
        std::vector<Chunk<T>*> chunkList;

        std::recursive_mutex mutex;
    };
}
