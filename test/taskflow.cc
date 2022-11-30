#include <iostream>

#include "gtest/gtest.h"

#include <taskflow.hpp>

namespace
{
    int spawn(int n, tf::Subflow& sbf)
    {
        if (n < 2) return n;
        int res1, res2;

        // compute f(n-1)
        sbf.emplace([&res1, n] (tf::Subflow& sbf) { res1 = spawn(n - 1, sbf); } )
            .name(std::to_string(n-1));

        // compute f(n-2)
        sbf.emplace([&res2, n] (tf::Subflow& sbf) { res2 = spawn(n - 2, sbf); } )
            .name(std::to_string(n-2));

        sbf.join();
        return res1 + res2;
    }
}


// ----------------------------------------------------------------------------------------
// ---------------------------        Test separator        -------------------------------
// ----------------------------------------------------------------------------------------
TEST(taskflow_test, fibonacci)
{
    constexpr unsigned int N = 15;

    int res;  // result

    tf::Executor executor;
    tf::Taskflow taskflow("fibonacci");

    taskflow.emplace([&res, N] (tf::Subflow& sbf) {
        res = spawn(N, sbf);
    }).name(std::to_string(N));

    executor.run(taskflow).wait();

    //taskflow.dump(std::cout);

    std::cout << "Fib[" << N << "]: " << res << std::endl;

}