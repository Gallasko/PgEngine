#include "uniqueid.h"

#include <atomic>

namespace pg
{
    namespace ecs
    {
        
        _unique_id generateId()
        {
            static std::atomic<_unique_id> uniqueId{3};

            return uniqueId++;
        }
    }
}