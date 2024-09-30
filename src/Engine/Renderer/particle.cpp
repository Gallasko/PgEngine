// #include "particle.h"

// namespace pg
// {
//     template <>
//     void renderer(MasterRenderer* masterRenderer, ParticleComponent* particle)
//     {
//         auto rTable = masterRenderer->getParameter();
//         const int screenWidth = rTable.get<int>("ScreenWidth");
//         const int screenHeight = rTable.get<int>("ScreenHeight");

//         auto extraFunctions = masterRenderer->getExtraFunctions();

//         auto instanceVBO = particle->instanceVBO;

//         QMatrix4x4 projection;
//         QMatrix4x4 view;
//         QMatrix4x4 model;
//         QMatrix4x4 scale;

//         auto particleShaderProgram = masterRenderer->getShader("particle");

//         particleShaderProgram->bind();

//         glBindTexture(GL_TEXTURE_2D, particle->texture);

//         projection.setToIdentity();
//         view.setToIdentity();
//         scale.setToIdentity();
//         model.setToIdentity();

//         scale.scale(QVector3D(1.0f / screenWidth, 1.0f / screenHeight, 0.0f));
//         view.translate(QVector3D(-1.0f, 1.0f, 0.0f));

//         particleShaderProgram->setUniformValue(particleShaderProgram->uniformLocation("projection"), projection);
//         particleShaderProgram->setUniformValue(particleShaderProgram->uniformLocation("view"), view);
//         particleShaderProgram->setUniformValue(particleShaderProgram->uniformLocation("model"), model);
//         particleShaderProgram->setUniformValue(particleShaderProgram->uniformLocation("scale"), scale);

//         particle->openglObject.VAO->bind();

//         instanceVBO->bind();
//         instanceVBO->setUsagePattern(QOpenGLBuffer::StreamDraw);
//         instanceVBO->allocate(particle->particleList, particle->count * sizeof(Particle));

//         // TODO the renderer can only draw 2D square particles !!
//         extraFunctions->glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, particle->count);
//     }
// }