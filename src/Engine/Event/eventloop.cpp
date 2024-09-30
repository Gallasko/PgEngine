#include "eventloop.h"

EventLoop::EventLoop()
{

}

int EventLoop::loop()
{
    do
    {
        std::unique_lock<std::mutex> lock(eventLoopMutex);
        eventLoopCV.wait(lock);

        if (not running)
            break;

        while (eventCount != 0)
        {
            for (auto const& dict : eventQueue)
            {
                // first : key, second : value
                if (dict.second.size() != 0)
                {
                    for (int i = dict.second.size(); i > 0; i--)
                    {
                        for (auto listener : eventMap[dict.first])
                        {
                            listener(dict.second[i - 1]);
                        }
                        eventQueue[dict.first].erase(eventQueue[dict.first].begin() + i - 1);
                        eventCount--;
                    }
                }
            }
        }

    } while (running);

    eventQueue.clear();
    eventMap.clear();

    return 0;
}