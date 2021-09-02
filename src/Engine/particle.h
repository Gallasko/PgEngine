#pragma once

#include <vector>
#include <functional>

#include "../constant.h"
#include "renderer.h"

struct Particle
{
    float lifetime;
    constant::Vector3D pos;
    constant::Vector2D texPos;
};

struct ParticleSubData {};

struct ParticleComponent
{
    unsigned int count;
    unsigned int texture;
    std::function<void()> onTick;

    Particle *particleList;
    ParticleSubData **particleSubDataList;
};

struct ParticleMoveSubData : public ParticleSubData 
{
    ParticleMoveSubData(unsigned int timeAlive, unsigned int timeToLive, constant::Vector3D velocity, std::vector<constant::Vector2D> textureSeq, unsigned int textureChangeRate) : timeAlive(timeAlive), timeToLive(timeToLive), velocity(velocity), textureSeq(textureSeq), textureChangeRate(textureChangeRate) {}
    
    unsigned int timeAlive;
    unsigned int timeToLive;
    constant::Vector3D velocity;
    std::vector<constant::Vector2D> textureSeq;
    unsigned int textureChangeRate;
};

struct ParticleRenderer : public Renderer
{
    using Renderer::Renderer;
    virtual ~ParticleRenderer() {}

    void render(MasterRenderer* masterRenderer...);
};


