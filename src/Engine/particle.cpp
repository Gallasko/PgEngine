#include "particle.h"

#include <iostream>

#include <QMatrix4x4>
#include <cstdarg>
void ParticleRenderer::render(MasterRenderer* masterRenderer...)
{ 
    va_list args; 
    va_start(args, masterRenderer); 
    auto particle = va_arg(args, ParticleComponent*);
    va_end(args);

    auto rTable = masterRenderer->getParameter();
    const int screenWidth = rTable["ScreenWidth"];
    const int screenHeight = rTable["ScreenHeight"];

    auto extraFunctions = masterRenderer->getExtraFunctions();
    //auto instanceVBO = masterRenderer->getInstanceVBO(); // TODO: implement this directly in the particle component Class
    auto instanceVBO = particle->instanceVBO; // TODO: implement this directly in the particle component Class
    //auto instanceVBO = masterRenderer->getInstanceVBO();

    QMatrix4x4 projection;
    QMatrix4x4 view;
    QMatrix4x4 model;
    QMatrix4x4 scale;

    auto particleShaderProgram = masterRenderer->getShader("particle");

    particleShaderProgram->bind();

    glBindTexture(GL_TEXTURE_2D, particle->texture);
    
    //std::cout << glGetError() << std::endl; 

    projection.setToIdentity();
    view.setToIdentity();
    scale.setToIdentity();
    model.setToIdentity();

    //scale.scale(QVector3D(2.0f, 2.0f, 0.0f));
    scale.scale(QVector3D(1.0f / screenWidth, 1.0f / screenHeight, 0.0f));
    view.translate(QVector3D(-1.0f, 1.0f, 0.0f));

    particleShaderProgram->setUniformValue(particleShaderProgram->uniformLocation("projection"), projection);
    particleShaderProgram->setUniformValue(particleShaderProgram->uniformLocation("view"), view);
    particleShaderProgram->setUniformValue(particleShaderProgram->uniformLocation("model"), model);
    particleShaderProgram->setUniformValue(particleShaderProgram->uniformLocation("scale"), scale);

    //std::cout << glGetError() << std::endl;

    particle->openglObject.VAO->bind();

    //std::cout << glGetError() << std::endl; 

    //for(int i = 0; i < particle->count; i++)
    //    std::cout << particle->particleList[i].lifetime;
    //std::cout << std::endl;

    //std::cout << particle->particleList[0].lifetime << std::endl;

    //for(int i = 0; i < particle->count; i++)
    //    std::cout << particle->particleList[i].pos.x << ", " << particle->particleList[i].pos.y << ", " << particle->particleList[i].pos.z << std::endl;
    
    instanceVBO->bind();
    instanceVBO->setUsagePattern(QOpenGLBuffer::StreamDraw);
    instanceVBO->allocate(&particle->particleList[0], particle->count * sizeof(Particle));

    //glEnableVertexAttribArray(2);
    //glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)0);
//
    //glEnableVertexAttribArray(3);
    //glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, pos));
//
    ////TODO check if we can send the tex vertex only once and not twice : once here and the second time in the squareVAO implementation 
    //glEnableVertexAttribArray(4);
    //glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, texOffset));
    //
    //extraFunctions->glVertexAttribDivisor(2, 1);
    //extraFunctions->glVertexAttribDivisor(3, 1);
    //extraFunctions->glVertexAttribDivisor(4, 1);

    //instanceVBO->allocate(particle->count * sizeof(Particle));
    //instanceVBO->write()

    //glBindBuffer(GL_ARRAY_BUFFER, particle->instanceVBO);
    //glBufferData(GL_ARRAY_BUFFER, particle->count * sizeof(Particle), NULL, GL_STREAM_DRAW);
    //glBufferSubData(GL_ARRAY_BUFFER, 0, particle->count * sizeof(Particle), &particle->particleList);

    //std::cout << glGetError() << std::endl; 

    extraFunctions->glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, particle->count);

    //std::cout << glGetError() << std::endl;  
}