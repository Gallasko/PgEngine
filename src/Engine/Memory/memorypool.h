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
     * @tparam N Number of object to allocate at once
     */
    template<typename T, size_t N>
    struct Block
    {
        /**
         * @brief Construct a new Block object
         * 
         * Create N free object and then passed the ownership of those element to the pool
         */
        Block()
        {
            // Allocate at once enough space for N objects
            chunks = new Chunk<T>[N];

            // Construct the free list of objects
            for(unsigned int i = 0; i < N - 1; i++)
            {
                chunks[i].next = &chunks[i + 1];
            }

            // End the free list
            chunks[N - 1].next = nullptr;
        }

        /** The first free space of the newly created block */
        Chunk<T>* chunks;
    };

    /**
     * @brief An implementation of an allocator pool
     * 
     * @tparam T Type of the object to be created
     * @tparam N Number of object to be created at once when running out of empty element (default at 64)
     */
    template<typename T, size_t N = 64>
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
            while(freeList != nullptr)
            {
                auto curr = freeList;
                freeList = curr->next;
                delete curr;
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
        T* allocate(const Args&... args)
        {
            if(freeList == nullptr)
            {
                auto newBlock = Block<T, N>();
                freeList = newBlock.chunks;
            }

            auto chunk = freeList;
            freeList = chunk->next;

            ::new(&(chunk->element)) T(args...);

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
        /** Pointer to the next free object in the pool */
        Chunk<T>* freeList = nullptr;

        /** Static assertion to ensure that the pool must created block of at least two free object */
        static_assert(N >= 2, "BlockSize too small.");
    };
}
