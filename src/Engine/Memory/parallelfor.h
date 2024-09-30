#pragma once

#include <algorithm>
#include <thread>
#include <functional>
#include <vector>

namespace pg
{
    /**
     * @param[in] nb_elements : size of your for loop
     * @param[in] functor(start, end): Your function processing a sub chunk of the for loop.
     * @param use_threads : enable / disable threads.
     * "start" is the first index to process (included) until the index "end" (excluded)
     */
    void parallelFor(const size_t& nb_elements, std::function<void (size_t start, size_t end)> functor, bool use_threads = true);
}