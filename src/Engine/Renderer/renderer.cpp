#define STB_IMAGE_IMPLEMENTATION

#include "renderer.h"

#include <filesystem>
namespace fs = std::filesystem;

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "logger.h"

#include "openglobject.h"

#include "Loaders/stb_image.h"

#include "UI/uisystem.h"

namespace pg
{
    namespace
    {
        constexpr static const char * const DOM = "Renderer";
    }

    AbstractRenderer::AbstractRenderer(MasterRenderer *masterRenderer, const RenderStage& stage) : masterRenderer(masterRenderer), renderStage(stage)
    {
        LOG_THIS_MEMBER("AbstractRenderer");

        masterRenderer->addRenderer(this);
    }

    void renderer(MasterRenderer* masterRenderer, const std::map<std::string, std::map<std::string, std::vector<RenderableTexture>>>& renderableTextureMap)
    {
        LOG_THIS(DOM);
    
        auto rTable = masterRenderer->getParameter();
        const int screenWidth = rTable["ScreenWidth"];
        const int screenHeight = rTable["ScreenHeight"];

        glm::mat4 projection;
        glm::mat4 view;
        glm::mat4 model;
        glm::mat4 scale;

        // glm::scale(scale, glm::vec3(0.5f, 0.5f, 0.0f));        

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

                shaderProgram->setUniformValue("texture1", 0);

                //glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, texture);

                // Todo combine all the call to the same texture into a single draw call using instanced rendering
                for(const auto& renderableTexture : textureMap.second)
                {
                    UiComponent *ui = renderableTexture.uiRef;

                    auto mesh = renderableTexture.meshRef;

                    if(not ui->isVisible() or not mesh)
                        continue;

                    // Todo
                    // view.translate(QVector3D(-1.0f + 2.0f * static_cast<UiSize>(ui->pos.x) / screenWidth, 1.0f + 2.0f * -static_cast<UiSize>(ui->pos.y) / screenHeight, -static_cast<UiSize>(ui->pos.z)));
                    glm::translate(view, glm::vec3(-1.0f + 2.0f * static_cast<UiSize>(ui->pos.x) / screenWidth, 1.0f + 2.0f * -static_cast<UiSize>(ui->pos.y) / screenHeight, 0.0f));
                    // glm::translate(view, glm::vec3(-0.5f , 0.5f, 1.0f));

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
        LOG_THIS_MEMBER(DOM);

        initializeParameters();
    }

    MasterRenderer::~MasterRenderer()
    {
        LOG_THIS_MEMBER(DOM);
    }

    void MasterRenderer::execute()
    {
        LOG_THIS_MEMBER(DOM);
 
        // Todo Fix in group and ecs ! ( whereaver we are holding pointer of a comp actually ! )
        // Todo hold a ref to the component list and the component index inside of this list instead of the raw pointer to not get invalidated on resize !

        std::lock_guard<std::mutex> lock(resizeMutex);

        for (auto renderer : renderers)
        {
            renderer->updateMeshes();
        }   
    }

    void MasterRenderer::renderAll()
    {
        LOG_THIS_MEMBER(DOM);

        std::lock_guard<std::mutex> lock(resizeMutex);

        for (auto renderer : renderers)
        {
            renderer->render();
        }   

        nbRenderedFrames++;
    }

    void MasterRenderer::registerShader(const std::string& name, OpenGLShaderProgram *shaderProgram)
    {
        LOG_THIS_MEMBER(DOM);

        shaderList[name] = shaderProgram;
    }

    void MasterRenderer::registerShader(const std::string& name, const char* vsPath, const char* fsPath)
    {
        LOG_THIS_MEMBER(DOM);

        auto shaderProgram = new OpenGLShaderProgram(vsPath, fsPath);

        registerShader(name, shaderProgram);
    }

    //TODO mirror or not the texture
    void MasterRenderer::registerTexture(const std::string& name, const char* texturePath)
    { 
        LOG_THIS_MEMBER(DOM);

        int width, height, nrChannels;
        // Todo change this with our custom file opener  (stbi_load_from_memory)

        unsigned char *data = stbi_load(texturePath, &width, &height, &nrChannels, STBI_rgb_alpha);

        if (not data)
        {
            if(stbi_failure_reason())
            {
                LOG_ERROR(DOM, "Failed to load texture: " << stbi_failure_reason());
            }
            else
            {
                LOG_ERROR(DOM, "Failed to load texture: Unknown");
            }

            return;
        }

        LOG_INFO(DOM, "Loaded texture " << name << " from " << texturePath << " with width = " << width << " height = " << height);

        unsigned int texture;

        glGenTextures(1, &texture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
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

    void MasterRenderer::initializeParameters()
    {
        LOG_THIS_MEMBER(DOM);

        systemParameters["ScreenWidth"] = 1;
        systemParameters["ScreenHeight"] = 1;
        systemParameters["CurrentTime"] = 1;
    }
}