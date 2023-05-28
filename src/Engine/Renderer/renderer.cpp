#define STB_IMAGE_IMPLEMENTATION

#include "renderer.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "UI/texture.h"

#include "logger.h"

#include "openglobject.h"

#include "Loaders/stb_image.h"

namespace pg
{
    namespace
    {
        constexpr static const char * const DOM = "Renderer";
    }

    void renderer(MasterRenderer* masterRenderer, const std::map<std::string, std::map<std::string, std::vector<RenderableTexture>>>& renderableTextureMap)
    {
        LOG_THIS(DOM);
    
        auto rTable = masterRenderer->getParameter();
        const int screenWidth = rTable["ScreenWidth"];
        const int screenHeight = rTable["ScreenHeight"];

        glm::mat4 projection(1.0);
        glm::mat4 view(1.0);
        glm::mat4 model(1.0);
        glm::mat4 scale(1.0);

        glm::scale(scale, glm::vec3(2.0f / screenWidth, 2.0f / screenHeight, 0.0f));

        // TODO why does it need to be scale * 2 ( the scaling now happen in the shader ) <- Done the * 2 is needed to map the -1 <-> 1 space to a 0 <-> 1 space 
        // Need to make a note about that

        for(const auto& shaderMap : renderableTextureMap)
        {
            auto shaderProgram = masterRenderer->getShader(shaderMap.first);

            for(const auto& textureMap : shaderMap.second)
            {
                auto texture = masterRenderer->getTexture(textureMap.first);

                // Tex rendering
                shaderProgram->bind();

                shaderProgram->setUniformValue("projection", projection);
                shaderProgram->setUniformValue("model", model);
                shaderProgram->setUniformValue("scale", scale);

                shaderProgram->setUniformValue("time", static_cast<int>(0 % 314159));

                //glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, texture);

                // Todo combine all the call to the same texture into a single draw call using instanced rendering
                for(const auto& renderableTexture : textureMap.second)
                {
                    UiComponent *ui = renderableTexture.uiRef;

                    auto mesh = renderableTexture.meshRef.getMesh();

                    if(not ui->isVisible() or not mesh)
                        continue;

                    // Todo
                    // view.translate(QVector3D(-1.0f + 2.0f * static_cast<UiSize>(ui->pos.x) / screenWidth, 1.0f + 2.0f * -static_cast<UiSize>(ui->pos.y) / screenHeight, -static_cast<UiSize>(ui->pos.z)));
                    glm::translate(view, glm::vec3(-1.0f + 2.0f * static_cast<UiSize>(ui->pos.x) / screenWidth, 1.0f + 2.0f * -static_cast<UiSize>(ui->pos.y) / screenHeight, 0.0f));

                    shaderProgram->setUniformValue("view", view);

                    mesh->bind();
                    glDrawElements(GL_TRIANGLES, mesh->modelInfo.nbIndices, GL_UNSIGNED_INT, 0);
                }
            }

            shaderProgram->release();
        }    
    }

    MasterRenderer::MasterRenderer()
    {

    }

    MasterRenderer::~MasterRenderer()
    { 
        delete squareObject;
        delete instanceVBO;
    }

    void MasterRenderer::execute()
    {
        LOG_THIS_MEMBER(DOM);
 
        // Todo Fix in group and ecs ! ( whereaver we are holding pointer of a comp actually ! )
        // Todo hold a ref to the component list and the component index inside of this list instead of the raw pointer to not get invalidated on resize !

        if(changed)
        {
            std::lock_guard<std::mutex> lock(modificationMutex);

            std::lock_guard<std::mutex> lock2(renderMutex);

            currentRenderList = tempRenderList;

            changed = false;
        }        
    }

    void MasterRenderer::renderAll()
    {
        LOG_THIS_MEMBER(DOM);

        std::lock_guard<std::mutex> lock(renderMutex);

        render(currentRenderList);

        nbRenderedFrames++;
    }

    void MasterRenderer::registerShader(const std::string& name, OpenGLShaderProgram *shaderProgram)
    { 
        shaderList[name] = shaderProgram;
    }

    void MasterRenderer::registerShader(const std::string& name, const char* vsPath, const char* fsPath)
    {
        LOG_THIS_MEMBER(DOM);

        auto shaderProgram = new OpenGLShaderProgram(vsPath, fsPath);
        // shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, vsPath);
        // shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, fsPath);
        // shaderProgram->link();

        registerShader(name, shaderProgram);
    }

    //TODO mirror or not the texture
    void MasterRenderer::registerTexture(const std::string& name, const char* texturePath)
    { 
        LOG_THIS_MEMBER(DOM);

        int width, height, nrChannels;
        unsigned char *data = stbi_load(name.c_str(), &width, &height, &nrChannels, 0);
        if (not data)
        {
            LOG_ERROR(DOM, "Failed to load texture");

            return;
        }

        unsigned int texture;

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        // set the texture wrapping parameters
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // load image, create texture and generate mipmaps
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        registerTexture(name, texture);

        stbi_image_free(data);
    }

    void MasterRenderer::initializeGlObject(OpenGLContext *context)
    {
        LOG_THIS_MEMBER(DOM); 

        instanceVBO = new OpenGLBuffer(OpenGLBuffer::VertexBuffer);
        instanceVBO->create();

        squareObject = new OpenGLObject();
        squareObject->initialize();

        auto tileVertices = new float[20];

        //                 x                         y                         z                      texpos x                 texpos y
        tileVertices[0]  = 0.0f; tileVertices[1]  =  0.0f; tileVertices[2]  = 1.0f; tileVertices[3]  = 0.0f; tileVertices[4]  = 0.0f;   
        tileVertices[5]  = 1.0f; tileVertices[6]  =  0.0f; tileVertices[7]  = 1.0f; tileVertices[8]  = 1.0f; tileVertices[9]  = 0.0f;
        tileVertices[10] = 1.0f; tileVertices[11] = -1.0f; tileVertices[12] = 1.0f; tileVertices[13] = 0.0f; tileVertices[14] = 1.0f;
        tileVertices[15] = 0.0f; tileVertices[16] = -1.0f; tileVertices[17] = 1.0f; tileVertices[18] = 1.0f; tileVertices[19] = 1.0f;

        unsigned int nbTileVertices = 20;

        auto tileVerticesIndice = new unsigned int[6];

        tileVerticesIndice[0] = 0; tileVerticesIndice[1] = 1; tileVerticesIndice[2] = 2;
        tileVerticesIndice[3] = 1; tileVerticesIndice[4] = 2; tileVerticesIndice[5] = 3;

        unsigned int nbOfElements = 6;

        squareObject->VAO->bind();

        // position attribute
        
        squareObject->VBO->bind();
        squareObject->VBO->setUsagePattern(OpenGLBuffer::StaticDraw);
        squareObject->VBO->allocate(tileVertices, nbTileVertices * sizeof(float));

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

        // texture coord attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        squareObject->EBO->bind();
        squareObject->EBO->setUsagePattern(OpenGLBuffer::StaticDraw);
        squareObject->EBO->allocate(tileVerticesIndice, nbOfElements * sizeof(unsigned int));

        squareObject->VAO->release();
    }

    void MasterRenderer::initializeParameters()
    {
        LOG_THIS_MEMBER(DOM);

        systemParameters["ScreenWidth"] = 1;
        systemParameters["ScreenHeight"] = 1;
        systemParameters["CurrentTime"] = 1;
    }
}