#include "uisystem.h"

UiComponent::UiComponent(const UiComponent& rhs)
{
    //TODO remove the previous reference of rhs inside the parent and push this pointer inside the parent child list to avoid resize error when this is being copied
    //also don t forget to call this constructor when creating the copy constructor of the child class
    this->visible = rhs.visible;
    this->pos = rhs.pos;
    this->width = rhs.width;
    this->height = rhs.height;
    this->scale = rhs.scale;
    this->topAnchor = rhs.topAnchor;
    this->rightAnchor = rhs.rightAnchor;
    this->bottomAnchor = rhs.bottomAnchor;
    this->leftAnchor = rhs.leftAnchor;

    this->top = &this->pos.y;
    this->right = this->pos.x + this->width;
    this->bottom = this->pos.y + this->height;
    this->left = &this->pos.x;

    //this->top = UiAnchor(this, &this->pos.y);
    //this->right = UiAnchor(this, &this->pos.x, &this->width);
    //this->bottom = UiAnchor(this, &this->pos.y, &this->height);
    //this->left = UiAnchor(this, &this->pos.x);
    this->topMargin = rhs.topMargin;
    this->rightMargin = rhs.rightMargin;
    this->bottomMargin = rhs.bottomMargin;
    this->leftMargin = rhs.leftMargin;
    this->children = rhs.children;
    this->scale = scale;
}

void UiComponent::update()
{
    if(topAnchor != nullptr && bottomAnchor != nullptr)
    {
        this->height = UiSize(0.0f, 1.0f, new UiSize(-bottomMargin, 1.0f, bottomAnchor),  new UiSize(-topMargin, 1.0f, topAnchor), UiSize::UiSizeOpType::SUB); // todo change this because () create elements that are temporary
        this->pos.y = *topAnchor + topMargin;
    }
    else if(topAnchor != nullptr && bottomAnchor == nullptr)
    {
        this->pos.y = *topAnchor + topMargin;
    }
    else if(topAnchor == nullptr && bottomAnchor != nullptr)
    {
        //this->pos.y = (*bottomAnchor - bottomMargin) - this->height;
        this->pos.y = UiSize(-this->height, 1.0f, new UiSize(-bottomMargin, 1.0f, bottomAnchor));
    }

    if(rightAnchor != nullptr && leftAnchor != nullptr)
    {
        this->width = UiSize(0.0f, 1.0f, new UiSize(-rightMargin, -1.0f, rightAnchor), new UiSize(-leftMargin, 1.0f, leftAnchor), UiSize::UiSizeOpType::SUB);
        this->pos.x = *leftAnchor + leftMargin;
    }
    else if(rightAnchor != nullptr && leftAnchor == nullptr)
    {
        this->pos.x = UiSize(this->width, 1.0f, new UiSize(-rightMargin, 1.0f, rightAnchor));
    }
    else if(rightAnchor == nullptr && leftAnchor != nullptr)
    {
        this->pos.x = *leftAnchor + leftMargin;
    }

    updated = true;

    //for(const auto& child : children)
    //    child->update();
}

TextureComponent::TextureComponent(UiSize width, UiSize height, const char* path) // TODO texture path should be fetch from the master renderer to avoid duplicate texture 
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

    //TODO dont create the texture inside the component but pull it from the masterRenderer 
    QImage textureAtlas = QImage(QString(path));
    textureAtlas = textureAtlas.convertToFormat(QImage::Format_RGBA8888);// mirrored();

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

TextureComponent::TextureComponent(const TextureComponent &rhs) : UiComponent(rhs)
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

    this->texture = rhs.texture;

    update();
}

TextureComponent::~TextureComponent()
{
    delete VAO;
	delete VBO;
	delete EBO;
}

#include <iostream>
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

void TextureRenderer::render(MasterRenderer* masterRenderer...)
{ 
    va_list args; 
    va_start(args, masterRenderer); 
    auto texture = va_arg(args, TextureComponent*);
    va_end(args);

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
    view.translate(QVector3D(-1.0f + 2.0f * (float)(texture->pos.x) / screenWidth, 1.0f + 2.0f * (float)( -texture->pos.y) / screenHeight, 0.0f));

    shaderProgram->setUniformValue(shaderProgram->uniformLocation("view"), view);

    texture->VAO->bind();
    glDrawElements(GL_TRIANGLES, texture->modelInfo.nbIndices, GL_UNSIGNED_INT, 0);

    shaderProgram->release();
}

void LoaderRenderer::render(MasterRenderer* masterRenderer...)
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