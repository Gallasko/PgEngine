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

namespace pg
{
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
            // Allocate at once enough space for N objects
            chunks = new Chunk<T>[size];

            // Construct the free list of objects
            for(unsigned int i = 0; i < size - 1; i++)
            {
                chunks[i].next = &chunks[i + 1];
            }

            // End the free list
            chunks[size - 1].next = nullptr;
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
            for(Chunk<T>* chunk : chunkList)
                delete chunk;
        }

        void reserve(size_t reserveSize)
        {
            std::lock_guard<std::mutex> lock(mutex);

            if(reserveSize < size)
                return;

            size_t blockSize = reserveSize - size;

            auto newBlock = Block<T>(blockSize);
            freeList = newBlock.chunks;

            chunkList.push_back(newBlock.chunks);

            size = reserveSize;
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
            if(freeList == nullptr)
            {
                size_t reserveSize = N >= 2 ? size + N : size * 2;

                reserve(reserveSize);
            }

            Chunk<T>* chunk;

            {
                std::lock_guard<std::mutex> lock(mutex);
                chunk = freeList;
                freeList = chunk->next;
            }

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
            if(pointer != nullptr)
            {
                pointer->~T();

                reinterpret_cast<Chunk<T>*>(pointer)->next = freeList;
                freeList = reinterpret_cast<Chunk<T>*>(pointer);
            }
        }

    private:
        size_t size = N >= 2 ? N : 1;

        /** Pointer to the next free object in the pool */
        Chunk<T>* freeList = nullptr;

        /** Chunk Lists used in the pool (used to free the memory) */
        std::vector<Chunk<T>*> chunkList;

        std::mutex mutex;

        /** Static assertion to ensure that the pool must created block of at least two free object */
        // static_assert(N >= 2, "BlockSize too small.");
    };
}
