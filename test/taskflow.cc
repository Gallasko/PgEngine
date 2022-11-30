#include <iostream>

#include "gtest/gtest.h"

#include <taskflow.hpp>
#include <algorithm/pipeline.hpp>

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

    struct MyObserver : public tf::ObserverInterface
    {
        MyObserver(const std::string& name) {
            std::cout << "constructing observer " << name << '\n';
        }

        // set_up is a constructor-like method that will be called exactly once
        // passing the number of workers
        void set_up(size_t num_workers) override final {
            std::cout << "setting up observer with " << num_workers << " workers\n";
        }

        // on_entry will be called before a worker runs a task
        void on_entry(tf::WorkerView wv, tf::TaskView tv) override final {
            std::ostringstream oss;
            oss << "worker " << wv.id() << " ready to run " << tv.name() << '\n';
            std::cout << oss.str();
        }

        // on_exit will be called after a worker completes a task
        void on_exit(tf::WorkerView wv, tf::TaskView tv) override final {
            std::ostringstream oss;
            oss << "worker " << wv.id() << " finished running " << tv.name() << '\n';
            std::cout << oss.str();
        }
    };
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

// ----------------------------------------------------------------------------------------
// ---------------------------        Test separator        -------------------------------
// ----------------------------------------------------------------------------------------
TEST(taskflow_test, composition)
{
    std::cout << "Composition example 2\n";

    tf::Executor executor;

    // f1 has two independent tasks
    tf::Taskflow f1("F1");
    auto f1A = f1.emplace([&](){ std::cout << "F1 TaskA\n"; });
    auto f1B = f1.emplace([&](){ std::cout << "F1 TaskB\n"; });
    f1A.name("f1A");
    f1B.name("f1B");

    //  f2A ---
    //         |----> f2C
    //  f2B ---
    //
    //  f1_module_task
    tf::Taskflow f2("F2");
    auto f2A = f2.emplace([&](){ std::cout << "  F2 TaskA\n"; });
    auto f2B = f2.emplace([&](){ std::cout << "  F2 TaskB\n"; });
    auto f2C = f2.emplace([&](){ std::cout << "  F2 TaskC\n"; });
    f2A.name("f2A");
    f2B.name("f2B");
    f2C.name("f2C");

    f2A.precede(f2C);
    f2B.precede(f2C);
    f2.composed_of(f1).name("module_of_f1");

    // f3 has a module task (f2) and a regular task
    tf::Taskflow f3("F3");
    f3.composed_of(f2).name("module_of_f2");
    f3.emplace([](){ std::cout << "      F3 TaskA\n"; }).name("f3A");

    // f4: f3_module_task -> f2_module_task
    tf::Taskflow f4;
    f4.name("F4");
    auto f3_module_task = f4.composed_of(f3).name("module_of_f3");
    auto f2_module_task = f4.composed_of(f2).name("module_of_f2");
    f3_module_task.precede(f2_module_task);

    f4.dump(std::cout);

    executor.run_until(
        f4,
        [iter = 1] () mutable { std::cout << '\n'; return iter-- == 0; },
        [](){ std::cout << "First run_until finished\n"; }
    ).get();

    executor.run_until(
        f4,
        [iter = 2] () mutable { std::cout << '\n'; return iter-- == 0; },
        [](){ std::cout << "Second run_until finished\n"; }
    );

    executor.run_until(
        f4,
        [iter = 3] () mutable { std::cout << '\n'; return iter-- == 0; },
        [](){ std::cout << "Third run_until finished\n"; }
    ).get();

}

// ----------------------------------------------------------------------------------------
// ---------------------------        Test separator        -------------------------------
// ----------------------------------------------------------------------------------------
TEST(taskflow_test, personal_composition)
{
    tf::Executor executor;

    // f1 has two independent tasks
    tf::Taskflow f1("F1");
    auto f1A = f1.emplace([&](){ std::cout << "F1 TaskA\n"; });
    auto f1B = f1.emplace([&](){ std::cout << "F1 TaskB\n"; });
    f1A.name("f1A");
    f1B.name("f1B");

    //  
    // f1_module_task ---> f2A ---> f2B ---> f2C
    //
    tf::Taskflow f2("F2");
    auto f2A = f2.emplace([&](){ std::cout << "- F2 TaskA\n"; });
    auto f2B = f2.emplace([&](){ std::cout << "- F2 TaskB\n"; });
    auto f2C = f2.emplace([&](){ std::cout << "- F2 TaskC\n"; });
    f2A.name("f2A");
    f2B.name("f2B");
    f2C.name("f2C");

    f2A.precede(f2B);
    f2B.precede(f2C);
    auto f1_module_task = f2.composed_of(f1).name("module_of_f1");
    f1_module_task.precede(f2A);

    // 
    //                             | ---> f3B
    // f2_module_task ---> f3A --- |
    //                             | ---> f3C
    //
    tf::Taskflow f3("F3");
    auto f2_module_task = f3.composed_of(f2).name("module_of_f2");
    auto f3A = f3.emplace([](){ std::cout << "- - F3 TaskA\n"; }).name("f3A");
    auto f3B = f3.emplace([](){ std::cout << "- - F3 TaskB\n"; }).name("f3B");
    auto f3C = f3.emplace([](){ std::cout << "- - F3 TaskC\n"; }).name("f3C");

    f2_module_task.precede(f3A);
    f3A.precede(f3B, f3C);

    f3.dump(std::cout);

    executor.run(f3).wait();
}

// ----------------------------------------------------------------------------------------
// ---------------------------        Test separator        -------------------------------
// ----------------------------------------------------------------------------------------
TEST(taskflow_test, pipeline)
{
    tf::Taskflow taskflow("pipeline");
    tf::Executor executor;

    const size_t num_lines = 4;

    // custom data storage
    std::array<int, num_lines> buffer;

    // the pipeline consists of three pipes (serial-parallel-serial)
    // and up to four concurrent scheduling tokens
    tf::Pipeline pl(num_lines,
        tf::Pipe{tf::PipeType::SERIAL, [&buffer](tf::Pipeflow& pf) {
        // generate only 5 scheduling tokens
        if(pf.token() == 5) {
            pf.stop();
        }
        // save the result of this pipe into the buffer
        else {
            printf("stage 1: input token = %zu\n", pf.token());
            buffer[pf.line()] = pf.token();
        }
        }},

        tf::Pipe{tf::PipeType::PARALLEL, [&buffer](tf::Pipeflow& pf) {
        printf(
            "stage 2: input buffer[%zu] = %d\n", pf.line(), buffer[pf.line()]
        );
        // propagate the previous result to this pipe and increment
        // it by one
        buffer[pf.line()] = buffer[pf.line()] + 1;
        }},

        tf::Pipe{tf::PipeType::SERIAL, [&buffer](tf::Pipeflow& pf) {
        printf(
            "stage 3: input buffer[%zu] = %d\n", pf.line(), buffer[pf.line()]
        );
        // propagate the previous result to this pipe and increment
        // it by one
        buffer[pf.line()] = buffer[pf.line()] + 1;
        }}
    );

    // build the pipeline graph using composition
    tf::Task init = taskflow.emplace([](){ std::cout << "ready\n"; })
                            .name("starting pipeline");
    tf::Task task = taskflow.composed_of(pl)
                            .name("pipeline");
    tf::Task stop = taskflow.emplace([](){ std::cout << "stopped\n"; })
                            .name("pipeline stopped");

    // create task dependency
    init.precede(task);
    task.precede(stop);

    // dump the pipeline graph structure (with composition)
    taskflow.dump(std::cout);

    // run the pipeline
    executor.run(taskflow).wait();
}

// ----------------------------------------------------------------------------------------
// ---------------------------        Test separator        -------------------------------
// ----------------------------------------------------------------------------------------
TEST(taskflow_test, observer)
{
    tf::Executor executor;

    // Create a taskflow of eight tasks
    tf::Taskflow taskflow;

    taskflow.emplace([] () { std::cout << "1\n"; }).name("A");
    taskflow.emplace([] () { std::cout << "2\n"; }).name("B");
    taskflow.emplace([] () { std::cout << "3\n"; }).name("C");
    taskflow.emplace([] () { std::cout << "4\n"; }).name("D");
    taskflow.emplace([] () { std::cout << "5\n"; }).name("E");
    taskflow.emplace([] () { std::cout << "6\n"; }).name("F");
    taskflow.emplace([] () { std::cout << "7\n"; }).name("G");
    taskflow.emplace([] () { std::cout << "8\n"; }).name("H");

    // create a default observer
    std::shared_ptr<MyObserver> observer = executor.make_observer<MyObserver>("MyObserver");

    // run the taskflow
    executor.run(taskflow).get();

    // remove the observer (optional)
    executor.remove_observer(std::move(observer));
}