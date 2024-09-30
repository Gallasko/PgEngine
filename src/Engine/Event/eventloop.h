#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <unordered_map>
#include <functional>
#include <typeinfo>
#include <vector>

#include <condition_variable>
#include <future>
#include <thread>
#include <mutex>

class Event
{
public:
    Event() {};
};

class EventLoop
{
public: 
    EventLoop();
    ~EventLoop() { stop(); }

    inline void start() { running = true; f = std::async(&EventLoop::loop, this); }
    inline void stop() { running = false; eventLoopCV.notify_all(); }
    inline int wait() { return f.get(); }
    int loop();

    template <typename EventType>    
    void registerEventType();

    template <typename EventType>
    void connectToEvent(std::function<void(EventType*)> func);

    template <typename EventType> 
    void queueEvent(EventType *event);

private:
    std::unordered_map<std::string, std::vector<std::function<void(Event*)>>> eventMap;
    std::unordered_map<std::string, std::vector<Event *>> eventQueue;
    bool running = false;
    int eventCount = 0;

    std::condition_variable eventLoopCV;
    std::mutex eventLoopMutex;
    std::future<int> f;
};

template <typename EventType>
void EventLoop::registerEventType()
{
    auto id = typeid(EventType).name();

    if (eventMap.find(id) == eventMap.end())
    {
        eventMap[id] = std::vector<std::function<void(Event*)>>();
        eventQueue[id] = std::vector<Event*>();
    }
}

template <typename EventType>
void EventLoop::connectToEvent(std::function<void(EventType*)> func)
{
    auto id = typeid(EventType).name();

    if (eventMap.find(id) != eventMap.end())
    {
        eventMap[id].push_back(
            [func](Event* e) { func(static_cast<EventType*>(e)); } // Push a lambda expression that convert an event into the EventType
            );
    }
}

template <typename EventType>
void EventLoop::queueEvent(EventType *event)
{
    auto id = typeid(*event).name();

    if (eventMap.find(id) != eventMap.end())
    {
        eventQueue[id].push_back(event);
        eventCount++;
        eventLoopCV.notify_one();
    }
}

#endif