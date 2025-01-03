#pragma once

#include "texture.h"

#include "Systems/coresystems.h"

namespace pg
{
    struct Animation2DKeyPoint
    {
        Animation2DKeyPoint(size_t timestamp, const Texture2DComponent& comp, CallablePtr callback = nullptr) : timestamp(timestamp), component(comp), callback(callback) { }
        Animation2DKeyPoint(const Animation2DKeyPoint& other) : timestamp(other.timestamp), component(other.component), callback(other.callback) { }

        Animation2DKeyPoint& operator=(const Animation2DKeyPoint& other)
        {
            timestamp = other.timestamp;
            component = other.component;

            return *this;
        }

        size_t timestamp;

        Texture2DComponent component;

        CallablePtr callback = nullptr;
    };

    struct Texture2DAnimationComponent
    {
        Texture2DAnimationComponent(const std::vector<Animation2DKeyPoint>& keypoints, bool runningOnStartup = true, bool loop = false) : running(runningOnStartup), keypoints(keypoints), looping(loop) {}
        Texture2DAnimationComponent(const Texture2DAnimationComponent& other) : running(other.running), elapsedTime(other.elapsedTime), startId(other.startId), keypoints(other.keypoints), looping(other.looping) {}

        void start() { startId = -1; elapsedTime = 0; running = true; }

        bool running = true;

        size_t elapsedTime = 0;
        int startId = -1;

        std::vector<Animation2DKeyPoint> keypoints;

        bool looping = false;
    };

    struct Texture2DAnimatorSystem : public System<Own<Texture2DAnimationComponent>, Listener<TickEvent>, InitSys>
    {
        virtual void init() override
        {
            registerGroup<Texture2DComponent, Texture2DAnimationComponent>();
        }

        virtual void onEvent(const TickEvent& event) override
        {
            deltaTime += event.tick;
        }

        virtual void execute() override
        {
            if (not deltaTime)
                return;

            for (const auto& elem : viewGroup<Texture2DComponent, Texture2DAnimationComponent>())
            {
                auto anim = elem->get<Texture2DAnimationComponent>();

                if (not anim->running)
                    continue;

                if (static_cast<size_t>(anim->startId + 1) >= anim->keypoints.size())
                {
                    if (anim->looping and anim->keypoints.size() > 0)
                    {
                        anim->startId = -1;
                        anim->elapsedTime = 0;
                    }
                    else
                    {
                        anim->running = false;
                        continue;
                    }
                }

                anim->elapsedTime += deltaTime;

                if (anim->elapsedTime >= anim->keypoints.at(anim->startId + 1).timestamp)
                {
                    anim->startId++;
                    auto tex = elem->get<Texture2DComponent>();

                    // *tex = anim->keypoints.at(anim->startId).component;
                    *tex.component = anim->keypoints.at(anim->startId).component;
                    // tex->setTexture(anim->keypoints.at(anim->startId).component.textureName);

                    if (anim->keypoints.at(anim->startId).callback)
                        anim->keypoints.at(anim->startId).callback->call(ecsRef);
                }
            }

            deltaTime = 0;
        }

        size_t deltaTime = 0;
    };

}