#pragma once

#include <cstdint>
#include <mutex>

namespace pg
{
    namespace ecs
    {
        typedef uint_fast64_t _unique_id;
        typedef uint_fast32_t _uint32;

        class UniqueIdGenerator
        {
        public:
            struct UniqueIdList
            {
                _unique_id start, end, length;
            };

        public:
            /**
             * @brief Generate an unique identifier
             * 
             * @return _unique_id An unique 64bit identifier
             * 
             * This function generates an unique identifier each time it is called.
             * It is thread safe and should always return a new number each time.
             * 
             * The first valid id is 3 as 0 is reserved for NONE, 1 is reserved for Ecs ID and 2 is reserved for Ecs Name,
             * which are the basic id of the ecs. 
             */
            _unique_id generateId() 
            {
                return currentId++;
            }

            UniqueIdList generateIdList(_unique_id size)
            {
                return {currentId, currentId += size, size};
            }

            _unique_id generateIdThreaded() 
            {
                std::lock_guard lock(m);
                return currentId++;
            }

            UniqueIdList generateIdListThreaded(_unique_id size)
            {
                std::lock_guard lock(m);
                return {currentId, currentId += size, size};
            }

        private:
            _unique_id currentId = 3;
            std::mutex m;
        };

        _unique_id generateId();
    }
}