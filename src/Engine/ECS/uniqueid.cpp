#include "uniqueid.h"

#include <mutex>

namespace pg
{
    namespace ecs
    {
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
            static _unique_id uniqueId = 3;
            static std::mutex mutex;

            std::lock_guard<std::mutex> lock(mutex);
            return uniqueId++;
        }
    }
}