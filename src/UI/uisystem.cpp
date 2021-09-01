#include "uisystem.h"

template<typename Type>
Type operator+(const Type& lhs, const UiSize& rhs)
{
    return lhs + UiSize::returnCurrentSize(&rhs);
}

template<typename Type>
Type operator-(const Type& lhs, const UiSize& rhs)
{
    return lhs - UiSize::returnCurrentSize(&rhs);
}

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

TextureComponent::TextureComponent(UiSize width, UiSize height, const char* path)
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
    this->top = rhs.top;
    this->right = rhs.right;
    this->bottom = rhs.bottom;
    this->left = rhs.left;
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
    static float oldWidth = 0.0f, oldheight = 0.0f;

    if(oldWidth != width || oldheight != height)
        initialised = false;

    if(!initialised)
    {
        modelInfo.vertices[0] =  0.0f;  modelInfo.vertices[1] =    0.0f; modelInfo.vertices[2] =  0.0f;
        modelInfo.vertices[5] =  width; modelInfo.vertices[6] =    0.0f; modelInfo.vertices[7] =  0.0f;
        modelInfo.vertices[10] = 0.0f;  modelInfo.vertices[11] =  -height; modelInfo.vertices[12] = 0.0f;
        modelInfo.vertices[15] = width; modelInfo.vertices[16] =  -height; modelInfo.vertices[17] = 0.0f;

        oldWidth = width;
        oldheight = height;

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

#include <QMatrix4x4>
#include <cstdarg>
void TextureRenderer::render(std::string rendererName...)
{ 
    va_list args; 
    va_start(args, rendererName); 
    auto screenWidth = va_arg(args, int);
    auto screenHeight = va_arg(args, int);
    auto texture = va_arg(args, TextureComponent*);
    va_end(args);

    QMatrix4x4 projection;
    QMatrix4x4 view;
    QMatrix4x4 model;
    QMatrix4x4 scale;

    projection.setToIdentity();
    model.setToIdentity();
    scale.setToIdentity();
    scale.scale(QVector3D(2.0f / screenWidth, 2.0f / screenHeight, 0.0f));

    if(shaderProgram == nullptr)
        return;

    // Tex rendering
    
    shaderProgram->bind();

    shaderProgram->setUniformValue(shaderProgram->uniformLocation("projection"), projection);
    shaderProgram->setUniformValue(shaderProgram->uniformLocation("model"), model);
    shaderProgram->setUniformValue(shaderProgram->uniformLocation("scale"), scale);

    //TODO gl scissor for list views 
    //glEnable(GL_SCISSOR_TEST);
    //glScissor(300, 200, 200, 500);

    texture->generateMesh();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->texture);

    view.setToIdentity();
    view.translate(QVector3D(-1.0f + 2.0f * (float)(texture->x) / screenWidth, 1.0f + 2.0f * (float)( -texture->y) / screenHeight, 0.0f));

    shaderProgram->setUniformValue(shaderProgram->uniformLocation("view"), view);

    texture->VAO->bind();
    glDrawElements(GL_TRIANGLES, texture->modelInfo.nbIndices, GL_UNSIGNED_INT, 0);

    shaderProgram->release();
}

void LoaderRenderer::render(std::string rendererName...)
{
    /*
    va_list args; 
    va_start(args, rendererName); 
    auto screenWidth = va_arg(args, int);
    auto screenHeight = va_arg(args, int);
    auto tileWidth = va_arg(args, int);
    auto tileHeight = va_arg(args, int);
    auto texture = va_arg(args, LoaderRenderer*);
    va_end(args);

    QMatrix4x4 projection;
    QMatrix4x4 view;
    QMatrix4x4 model;
    QMatrix4x4 scale;

    projection.setToIdentity();
    model.setToIdentity();
    scale.setToIdentity();
    scale.scale(QVector3D(2.0f / screenWidth, 2.0f / screenHeight, 0.0f));

    // Text rendering

    shaderProgram->bind();

    shaderProgram->setUniformValue(shaderProgram->uniformLocation("projection"), projection);
    shaderProgram->setUniformValue(shaderProgram->uniformLocation("model"), model);

    texture->generateMesh();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->texture);

    view.setToIdentity();
    view.translate(QVector3D(-1.0f + 2.0f * (float)(texture->x) / screenWidth, 1.0f + 2.0f * (float)( -texture->y) / screenHeight, 0.0f));

    defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("view"), view);

    texture->VAO->bind();
    glDrawElements(GL_TRIANGLES, texture->modelInfo.nbIndices, GL_UNSIGNED_INT, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tileTexture);

    scale.setToIdentity();
    scale.scale(QVector3D(tileWidth / screenWidth, tileHeight / screenHeight, 0.0f));
    shaderProgram->setUniformValue(shaderProgram->uniformLocation("scale"), scale);

    for(auto tile : tileRendererVector)
    {
        view.setToIdentity();
        view.translate(QVector3D(-1.0f + 2.0f * (float)(tile.x + (tileHeight / 4.0f)) / screenWidth, 1.0f + 2.0f * (float)( -tile.y - (tileHeight / 8.0f)) / screenHeight, 0.0f));

        shaderProgram->setUniformValue(shaderProgram->uniformLocation("view"), view);

        tile.id->getMesh()->bind();
        glDrawElements(GL_TRIANGLES, tile.id->getModelInfo().nbIndices, GL_UNSIGNED_INT, 0);
    }

    //glDisable(GL_SCISSOR_TEST);

    shaderProgram->release();
    */
}