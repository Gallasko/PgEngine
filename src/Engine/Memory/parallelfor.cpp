#include "parallelfor.h"

namespace pg
{
    void parallelFor(const size_t& nb_elements, std::function<void (size_t start, size_t end)> functor, bool use_threads)
    {
        // -------
        unsigned nb_threads_hint = std::thread::hardware_concurrency();
        unsigned nb_threads = nb_threads_hint == 0 ? 8 : (nb_threads_hint);

        size_t batch_size = nb_elements / nb_threads;
        size_t batch_remainder = nb_elements % nb_threads;

        std::vector< std::thread > my_threads(nb_threads);

        if( use_threads )
        {
            // Multithread execution
            for(size_t i = 0; i < nb_threads; ++i)
            {
                size_t start = i * batch_size;
                my_threads[i] = std::thread(functor, start, start+batch_size);
            }
        }
        else
        {
            // Single thread execution (for easy debugging)
            for(size_t i = 0; i < nb_threads; ++i){
                size_t start = i * batch_size;
                functor( start, start+batch_size );
            }
        }

        // Deform the elements left
        size_t start = nb_threads * batch_size;
        functor( start, start+batch_remainder);

        // Wait for the other thread to finish their task
        if( use_threads )
            std::for_each(my_threads.begin(), my_threads.end(), std::mem_fn(&std::thread::join));
    }

}