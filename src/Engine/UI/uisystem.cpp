#include "uisystem.h"

#include "../logger.h"
#include "../serialization.h"

namespace pg
{
    namespace
	{
		const char * DOM = "Ui System";
	}

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
    void serialize(Archive& archive, const UiSize& value)
    {
        LOG_THIS(DOM);

        archive.startSerialization("UiSize");

        serialize(archive, "value", static_cast<float>(value));

        archive.endSerialization();
    }

    template<>
    void serialize(Archive& archive, const UiPosition& value)
    {
        LOG_THIS(DOM);

        archive.startSerialization("UiPosition");

        serialize(archive, "x", value.x);
        serialize(archive, "y", value.y);
        serialize(archive, "z", value.z);

        archive.endSerialization();
    }

    template<>
    void serialize(Archive& archive, const UiFrame& value)
    {
        LOG_THIS(DOM);

        archive.startSerialization("UiFrame");

        serialize(archive, "pos", value.pos);
        serialize(archive, "w", value.w);
        serialize(archive, "h", value.h);

        archive.endSerialization();
    }

    template<>
    void serialize(Archive& archive, const UiComponent& value)
    {
        LOG_THIS(DOM);

        archive.startSerialization("UiComponent");

        serialize(archive, "visibility", value.isVisible());
        serialize(archive, "pos", value.pos);
        serialize(archive, "width", value.width);
        serialize(archive, "height", value.height);

        serialize(archive, "topMargin", value.topMargin);
        serialize(archive, "rightMargin", value.rightMargin);
        serialize(archive, "bottomMargin", value.bottomMargin);
        serialize(archive, "leftMargin", value.leftMargin);

        archive.endSerialization();
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
    UiSize deserialize(const UnserializedObject& serializedString)
    {
        LOG_THIS(DOM);

        UiSize size;

        std::string type = "";

        if(serializedString.isNull())
            LOG_ERROR(DOM, "Element is null");
        else
        {
            size = deserialize<float>(serializedString["value"]);
        }

        return size;
    }

    template<>
    UiPosition deserialize(const UnserializedObject& serializedString)
    {
        LOG_THIS(DOM);

        UiPosition pos;

        std::string type = "";

        if(serializedString.isNull())
            LOG_ERROR(DOM, "Element is null");
        else
        {
            pos.x = deserialize<UiSize>(serializedString["x"]);
            pos.y = deserialize<UiSize>(serializedString["y"]);
            pos.z = deserialize<UiSize>(serializedString["z"]);
        }

        return pos;
    }

    template<>
    UiFrame deserialize(const UnserializedObject& serializedString)
    {
        LOG_THIS(DOM);

        UiFrame frame;

        std::string type = "";

        if(serializedString.isNull())
            LOG_ERROR(DOM, "Element is null");
        else
        {
            LOG_INFO(DOM, "Deserializing an UiFrame");

            frame.pos = deserialize<UiPosition>(serializedString["pos"]);
            frame.w = deserialize<UiSize>(serializedString["w"]);
            frame.h = deserialize<UiSize>(serializedString["h"]);
        }

        return frame;
    }

    template<>
    UiComponent deserialize(const UnserializedObject& serializedString)
    {
        LOG_THIS(DOM);

        UiComponent component;

        std::string type = "";

        if(serializedString.isNull())
            LOG_ERROR(DOM, "Element is null");
        else
        {
            LOG_INFO(DOM, "Deserializing an UiComponent");

            deserialize<bool>(serializedString["visibility"]) ? component.show() : component.hide();

            component.pos = deserialize<UiPosition>(serializedString["pos"]);
            component.width = deserialize<UiSize>(serializedString["width"]);
            component.height = deserialize<UiSize>(serializedString["height"]);

            component.topMargin = deserialize<UiSize>(serializedString["topMargin"]);
            component.rightMargin = deserialize<UiSize>(serializedString["rightMargin"]);
            component.bottomMargin = deserialize<UiSize>(serializedString["bottomMargin"]);
            component.leftMargin = deserialize<UiSize>(serializedString["leftMargin"]);

            component.update();
        }

        return component;
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

    UiComponent::UiComponent(const UiComponent& rhs)
    {
        //TODO remove the previous reference of rhs inside the parent and push this pointer inside the parent child list to avoid resize error when this is being copied
        //also don t forget to call this constructor when creating the copy constructor of the child class
        this->visible = rhs.visible;
        this->pos = rhs.pos;
        this->width = rhs.width;
        this->height = rhs.height;
        this->topAnchor = rhs.topAnchor;
        this->rightAnchor = rhs.rightAnchor;
        this->bottomAnchor = rhs.bottomAnchor;
        this->leftAnchor = rhs.leftAnchor;

        this->topMargin = rhs.topMargin;
        this->rightMargin = rhs.rightMargin;
        this->bottomMargin = rhs.bottomMargin;
        this->leftMargin = rhs.leftMargin;
    }

    bool UiComponent::inBound(int x, int y) const
    {
        // Lockup x and y only once
        const float xValue = x;
        const float yValue = y;

        return xValue > this->pos.x && xValue < (this->pos.x + this->width) && yValue < (this->pos.y + this->height) && yValue > this->pos.y;
    }

    void UiComponent::render(MasterRenderer*)
    {
        LOG_ERROR(DOM, "Called Render of UiComponent when it should never be !");
    }

    void UiComponent::update()
    {
        if(topAnchor != nullptr && bottomAnchor != nullptr)
        {
            this->height = (*bottomAnchor - bottomMargin) - (*topAnchor - topMargin);
            //this->height = UiSize(0.0f, 1.0f, new UiSize(-bottomMargin, 1.0f, bottomAnchor), new UiSize(-topMargin, 1.0f, topAnchor), UiSize::UiSizeOpType::SUB); // todo change this because () create elements that are temporary
            this->pos.y = *topAnchor + topMargin;
        }
        else if(topAnchor != nullptr && bottomAnchor == nullptr)
        {
            this->pos.y = *topAnchor + topMargin;
        }
        else if(topAnchor == nullptr && bottomAnchor != nullptr)
        {
            //this->pos.y = (*bottomAnchor - bottomMargin) - this->height;
            this->pos.y = (*bottomAnchor - bottomMargin) - this->height;
            //this->pos.y = UiSize(-this->height, 1.0f, new UiSize(-bottomMargin, 1.0f, bottomAnchor));
        }

        if(rightAnchor != nullptr && leftAnchor != nullptr)
        {
            this->width = (*rightAnchor - rightMargin) - (*leftAnchor - leftMargin);
            //this->width = UiSize(0.0f, 1.0f, new UiSize(-rightMargin, -1.0f, rightAnchor), new UiSize(-leftMargin, 1.0f, leftAnchor), UiSize::UiSizeOpType::SUB);
            this->pos.x = *leftAnchor + leftMargin;
        }
        else if(rightAnchor != nullptr && leftAnchor == nullptr)
        {
            this->pos.x = (*rightAnchor - rightMargin) - this->width;
            //this->pos.x = UiSize(this->width, 1.0f, new UiSize(-rightMargin, 1.0f, rightAnchor));
        }
        else if(rightAnchor == nullptr && leftAnchor != nullptr)
        {
            this->pos.x = *leftAnchor + leftMargin;
        }
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
    //        std::cout << width << " " << height << std::endl;

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