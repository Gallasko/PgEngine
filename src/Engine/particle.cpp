#include "particle.h"

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

    auto instanceVBO = particle->instanceVBO;

    QMatrix4x4 projection;
    QMatrix4x4 view;
    QMatrix4x4 model;
    QMatrix4x4 scale;

    auto particleShaderProgram = masterRenderer->getShader("particle");

    particleShaderProgram->bind();

    glBindTexture(GL_TEXTURE_2D, particle->texture);

    projection.setToIdentity();
    view.setToIdentity();
    scale.setToIdentity();
    model.setToIdentity();

    scale.scale(QVector3D(1.0f / screenWidth, 1.0f / screenHeight, 0.0f));
    view.translate(QVector3D(-1.0f, 1.0f, 0.0f));

    particleShaderProgram->setUniformValue(particleShaderProgram->uniformLocation("projection"), projection);
    particleShaderProgram->setUniformValue(particleShaderProgram->uniformLocation("view"), view);
    particleShaderProgram->setUniformValue(particleShaderProgram->uniformLocation("model"), model);
    particleShaderProgram->setUniformValue(particleShaderProgram->uniformLocation("scale"), scale);

    particle->openglObject.VAO->bind();

    instanceVBO->bind();
    instanceVBO->setUsagePattern(QOpenGLBuffer::StreamDraw);
    instanceVBO->allocate(particle->particleList, particle->count * sizeof(Particle));

    extraFunctions->glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, particle->count);
}