#include "uisystem.h"

void UiComponent::update()
{
    if(topAnchor != nullptr && bottomAnchor != nullptr)
    {
        this->height = bottomAnchor->y - bottomMargin - topAnchor->y - topAnchor->height - topMargin;
        this->y = topAnchor->y + topAnchor->height + topMargin;
    }
    else if(topAnchor != nullptr && bottomAnchor == nullptr)
    {
        this->y = topAnchor->y + topAnchor->height + topMargin;
    }
    else if(topAnchor == nullptr && bottomAnchor != nullptr)
    {
        this->y = bottomAnchor->y - height - bottomMargin;
    }

    if(rightAnchor != nullptr && leftAnchor != nullptr)
    {
        this->width = rightAnchor->x - rightMargin - leftAnchor->x - leftAnchor->width - leftMargin;
        this->x = leftAnchor->x + leftAnchor->width + leftMargin;
    }
    else if(rightAnchor != nullptr && leftAnchor == nullptr)
    {
        this->x = rightAnchor->x - width - rightMargin;
    }
    else if(rightAnchor == nullptr && leftAnchor != nullptr)
    {
        this->x = leftAnchor->x + leftAnchor->width + leftMargin;
    }

    updated = true;

    for(auto child : children)
        child->update();
}

TextureComponent::TextureComponent(unsigned int width, unsigned int height, const char* path)
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

    QImage textureAtlas = QImage(QString(path));
    textureAtlas = textureAtlas.convertToFormat(QImage::Format_RGBA8888).mirrored();

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // load image, create texture and generate mipmaps
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureAtlas.width(), textureAtlas.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, textureAtlas.bits());
    glGenerateMipmap(GL_TEXTURE_2D);
}

TextureComponent::TextureComponent(const TextureComponent &rhs)
{
    initializeOpenGLFunctions(); 

    VAO = new QOpenGLVertexArrayObject();
	VBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	EBO = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);

	VAO->create();
	VBO->create();
	EBO->create();

    this->visible = rhs.visible;
    this->x = rhs.x;
    this->y = rhs.y;
    this->z = rhs.z;
    this->width = rhs.width;
    this->height = rhs.height;
    this->scale = rhs.scale;
    this->topAnchor = rhs.topAnchor;
    this->rightAnchor = rhs.rightAnchor;
    this->bottomAnchor = rhs.bottomAnchor;
    this->leftAnchor = rhs.leftAnchor;
    this->topMargin = rhs.topMargin;
    this->rightMargin = rhs.rightMargin;
    this->bottomMargin = rhs.bottomMargin;
    this->leftMargin = rhs.leftMargin;
    this->children = rhs.children;
    this->scale = scale;

    this->modelInfo = rhs.modelInfo;

    this->texture = rhs.texture;

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
    modelInfo.vertices[0] =  0.0f;  modelInfo.vertices[1] =    0.0f; modelInfo.vertices[2] =  0.0f;
    modelInfo.vertices[5] =  width; modelInfo.vertices[6] =    0.0f; modelInfo.vertices[7] =  0.0f;
    modelInfo.vertices[10] = 0.0f;  modelInfo.vertices[11] =  -height; modelInfo.vertices[12] = 0.0f;
    modelInfo.vertices[15] = width; modelInfo.vertices[16] =  -height; modelInfo.vertices[17] = 0.0f;

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