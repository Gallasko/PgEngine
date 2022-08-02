#pragma once

#include <cstdint>

namespace pg
{
    namespace ecs
    {
        typedef uint_fast64_t _unique_id;

        const _unique_id generateId();

        const _unique_id generateSystemId();
    }
}