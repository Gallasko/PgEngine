#pragma once

#include <mutex>

class RWLock
{
public:
    RWLock() : shared(), readerQ(), writerQ(), active_readers(0), waiting_writers(0), active_writers(0) {}

    void readLock()
    {
        std::unique_lock<std::mutex> lk(shared);
        while( waiting_writers != 0 )
            readerQ.wait(lk);
        ++active_readers;
        lk.unlock();
    }

    void readUnlock()
    {
        std::unique_lock<std::mutex> lk(shared);
        --active_readers;
        lk.unlock();
        writerQ.notify_one();
    }

    void writeLock()
    {
        std::unique_lock<std::mutex> lk(shared);
        ++waiting_writers;
        while( active_readers != 0 || active_writers != 0 )
            writerQ.wait(lk);
        ++active_writers;
        lk.unlock();
    }

    void writeUnlock()
    {
        std::unique_lock<std::mutex> lk(shared);
        --waiting_writers;
        --active_writers;
        if(waiting_writers > 0)
            writerQ.notify_one();
        else
            readerQ.notify_all();
        lk.unlock();
    }

private:
    std::mutex shared;
    std::condition_variable readerQ;
    std::condition_variable writerQ;

    int active_readers;
    int waiting_writers;
    int active_writers;
};