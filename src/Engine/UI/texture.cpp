#include "texture.h"

#include "logger.h"
#include "serialization.h"

#include "Renderer/renderer.h"

namespace pg
{
    static constexpr char const * DOM = "Texture";

    template<>
    void renderer(MasterRenderer* masterRenderer, TextureComponent* texture)
    {
        if(not texture->isVisible())
            return;
    
        auto rTable = masterRenderer->getParameter();
        const int screenWidth = rTable["ScreenWidth"];
        const int screenHeight = rTable["ScreenHeight"];

        QMatrix4x4 projection;
        QMatrix4x4 view;
        QMatrix4x4 model;
        QMatrix4x4 scale;

        projection.setToIdentity();
        model.setToIdentity();
        scale.setToIdentity();
        scale.scale(QVector3D(2.0f / screenWidth, 2.0f / screenHeight, 0.0f)); 
        // TODO why does it need to be scale * 2 ( the scaling now happen in the shader ) <- Done the * 2 is needed to map the -1 <-> 1 space to a 0 <-> 1 space 
        // Need to make a note about that

        auto shaderProgram = masterRenderer->getShader("default");
        auto tex = masterRenderer->getTexture(texture->textureName);

        // Tex rendering
        
        shaderProgram->bind();

        shaderProgram->setUniformValue(shaderProgram->uniformLocation("projection"), projection);
        shaderProgram->setUniformValue(shaderProgram->uniformLocation("model"), model);
        shaderProgram->setUniformValue(shaderProgram->uniformLocation("scale"), scale);

        texture->generateMesh();

        //glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);

        view.setToIdentity();
        view.translate(QVector3D(-1.0f + 2.0f * (float)(texture->pos.x) / screenWidth, 1.0f + 2.0f * (float)( -texture->pos.y) / screenHeight, 0.0f));

        shaderProgram->setUniformValue(shaderProgram->uniformLocation("view"), view);

        texture->VAO->bind();
        glDrawElements(GL_TRIANGLES, texture->modelInfo.nbIndices, GL_UNSIGNED_INT, 0);

        shaderProgram->release();
    }

    template<>
    void serialize(Archive& archive, const TextureComponent& value)
    {
        LOG_THIS(DOM);

        archive.startSerialization("TextureComponent");

        serialize(archive, "uicomponent", static_cast<UiComponent>(value));
        serialize(archive, "textureName", value.textureName);
    
        archive.endSerialization();
    }

    template<>
    TextureComponent deserialize(const UnserializedObject& serializedString)
    {
        LOG_THIS(DOM);

        std::string type = "";

        if(serializedString.isNull())
            LOG_ERROR(DOM, "Element is null");
        else
        {
            LOG_INFO(DOM, "Deserializing an TextureComponent");

            auto uiComponent = deserialize<UiComponent>(serializedString["uicomponent"]);
            auto textureName = deserialize<std::string>(serializedString["textureName"]);

            return TextureComponent{uiComponent, textureName};
        }

        return TextureComponent{0.0f, 0.0f, ""};
    }

    TextureComponent::TextureComponent(const UiSize& width, const UiSize& height, const std::string& textureName) : textureName(textureName)
    {
        initializeOpenGLFunctions(); 

        VAO = new QOpenGLVertexArrayObject();
        VBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        EBO = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);

        VAO->create();
        VBO->create();
        EBO->create();

        this->width = width;
        this->height = height;
    }

    TextureComponent::TextureComponent(const UiComponent& component, const std::string& textureName) : UiComponent(component), QOpenGLFunctions(), textureName(textureName)
    {
        initializeOpenGLFunctions(); 

        VAO = new QOpenGLVertexArrayObject();
        VBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        EBO = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);

        VAO->create();
        VBO->create();
        EBO->create();

        update();
    }

    TextureComponent::TextureComponent(const TextureComponent &rhs) : UiComponent(rhs), QOpenGLFunctions(), textureName(rhs.textureName)
    {
        initializeOpenGLFunctions(); 

        VAO = new QOpenGLVertexArrayObject();
        VBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        EBO = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);

        VAO->create();
        VBO->create();
        EBO->create();

        this->oldWidth = rhs.oldWidth;
        this->oldHeight = rhs.oldHeight;

        this->modelInfo = rhs.modelInfo;

        update();
    }

    TextureComponent::~TextureComponent()
    {
        delete VAO;
        delete VBO;
        delete EBO;
    }

    void TextureComponent::generateMesh()
    {
        if(oldWidth != width || oldHeight != height)
            initialised = false;

        if(!initialised)
        {
            // TODO fix this generateMesh() why we need a *2 to get correct size and why does it update itself each frame ?

            modelInfo.vertices[0] =  0.0f;  modelInfo.vertices[1] =    0.0f;   modelInfo.vertices[2] =  0.0f;
            modelInfo.vertices[5] =  width; modelInfo.vertices[6] =    0.0f;   modelInfo.vertices[7] =  0.0f;
            modelInfo.vertices[10] = 0.0f;  modelInfo.vertices[11] =  -height; modelInfo.vertices[12] = 0.0f;
            modelInfo.vertices[15] = width; modelInfo.vertices[16] =  -height; modelInfo.vertices[17] = 0.0f;

            oldWidth = width;
            oldHeight = height;

            VAO->bind();

            // position attribute
            VBO->bind();
            VBO->setUsagePattern(QOpenGLBuffer::StreamDraw);
            VBO->allocate(modelInfo.vertices, modelInfo.nbVertices * sizeof(float));

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

            // texture coord attribute
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

            EBO->bind();
            EBO->setUsagePattern(QOpenGLBuffer::StreamDraw);
            EBO->allocate(modelInfo.indices, modelInfo.nbIndices * sizeof(unsigned int));

            VAO->release();

            initialised = true;
        }
    }

    void TextureComponent::render(MasterRenderer* masterRenderer)
    { 
        renderer(masterRenderer, this); 
    }
}