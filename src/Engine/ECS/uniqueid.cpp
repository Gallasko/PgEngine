#include "uniqueid.h"

#include <mutex>

namespace pg
{
    namespace ecs
    {
        _unique_id generateId()
        {
            static _unique_id uniqueId = 0;
            static std::mutex mutex;

            std::lock_guard<std::mutex> lock(mutex);
            return uniqueId++;
        }

        _unique_id generateSystemId()
        {
            static _unique_id uniqueId = 0;
            static std::mutex mutex;

            std::lock_guard<std::mutex> lock(mutex);
            return uniqueId++;
        }
    }
}