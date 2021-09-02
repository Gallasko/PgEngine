#include "particle.h"

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
    auto squareVAO = masterRenderer->getSquareVAO();
    auto instanceVBO = masterRenderer->getInstanceVBO();

    QMatrix4x4 projection;
    QMatrix4x4 view;
    QMatrix4x4 model;
    QMatrix4x4 scale;

    projection.setToIdentity();
    view.setToIdentity();

    auto particleShaderProgram = masterRenderer->getShader("particle");
    //guiShaderProgram->bind();

    glBindTexture(GL_TEXTURE_2D, particle->texture);

    projection.setToIdentity();
    view.setToIdentity();
    scale.setToIdentity();
    //scale.scale(QVector3D(2.0f, 2.0f, 0.0f));
    scale.scale(QVector3D(1.0f / screenWidth, 1.0f / screenHeight, 0.0f));
    model.setToIdentity();
    view.translate(QVector3D(-1.0f, 1.0f, 0.0f));

    particleShaderProgram->setUniformValue(particleShaderProgram->uniformLocation("projection"), projection);
    particleShaderProgram->setUniformValue(particleShaderProgram->uniformLocation("view"), view);
    particleShaderProgram->setUniformValue(particleShaderProgram->uniformLocation("model"), model);
    particleShaderProgram->setUniformValue(particleShaderProgram->uniformLocation("scale"), scale);

    squareVAO->bind();

    instanceVBO->bind();
    instanceVBO->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    instanceVBO->allocate(particle->particleList, particle->count * sizeof(Particle));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(float), (void*)0);
    extraFunctions->glVertexAttribDivisor(2, 1);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)sizeof(float));
    extraFunctions->glVertexAttribDivisor(3, 1);

    //TODO check if we can send the tex vertex only once and not twice : once here and the second time in the squareVAO implementation 
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(sizeof(float) + 3 * sizeof(float)));
    extraFunctions->glVertexAttribDivisor(4, 1);

    extraFunctions->glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, particle->count);
}