#include "app.h"

GameWindow::GameWindow(QWindow *parent) : QWindow(parent)
{
    setSurfaceType(QWindow::OpenGLSurface);

    mousePos = QPoint(0.0f, 0.0f);

    inputHandler = new Input();

    constant::Vector3D vec;

    std::cout << "Taille d'un vec3: " << sizeof(vec) << " Taille de 3 floats: " << 3 * sizeof(float) << std::endl; 
}

GameWindow::~GameWindow()
{
    delete m_device;
}

void GameWindow::render(QPainter *painter)
{
    Q_UNUSED(painter);
}

void GameWindow::initialize()
{
	initializeOpenGLFunctions();
    extraFunctions = new QOpenGLExtraFunctions(m_context);

    //glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	defaultShaderProgram = new QOpenGLShaderProgram(this);
    defaultShaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, "shader/default.vs");
    defaultShaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, "shader/default.fs");
    defaultShaderProgram->link();

    guiShaderProgram = new QOpenGLShaderProgram(this);
    guiShaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, "shader/default.vs");
    guiShaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, "shader/default.fs");
    guiShaderProgram->link();

    textShaderProgram = new QOpenGLShaderProgram(this);
    textShaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, "shader/textrendering.vs");
    textShaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, "shader/textrendering.fs");
    textShaderProgram->link();

    // texture Atlas
    // ---------

    QImage textureAtlas = QImage(QString("res/tiles/TeclaEatsAtlas.png"));
    textureAtlas = textureAtlas.convertToFormat(QImage::Format_RGBA8888);

    glGenTextures(1, &baseTileTexture1);
    glBindTexture(GL_TEXTURE_2D, baseTileTexture1);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // load image, create texture and generate mipmaps
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureAtlas.width(), textureAtlas.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, textureAtlas.bits());
    glGenerateMipmap(GL_TEXTURE_2D);

    defaultShaderProgram->setUniformValue("texture1", baseTileTexture1);

    // texture Menu
    // ---------

    textureAtlas = QImage(QString("res/menu/Menu.png"));
    textureAtlas = textureAtlas.convertToFormat(QImage::Format_RGBA8888);

    glGenTextures(1, &baseMenu1);
    glBindTexture(GL_TEXTURE_2D, baseMenu1);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // load image, create texture and generate mipmaps
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureAtlas.width(), textureAtlas.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, textureAtlas.bits());
    glGenerateMipmap(GL_TEXTURE_2D);

    // texture Font
    // ---------

    textureAtlas = QImage(QString("res/font/font.png"));
    textureAtlas = textureAtlas.convertToFormat(QImage::Format_RGBA8888);

    glGenTextures(1, &fontTexture);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // load image, create texture and generate mipmaps
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureAtlas.width(), textureAtlas.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, textureAtlas.bits());
    glGenerateMipmap(GL_TEXTURE_2D);

    srand(time(0));

    for(int i = 0; i < nbMaxParticle; i++)
    {
        particleList[i].lifetime = 50.0f; // (rand() % 100) / 10.0f;
        particleList[i].pos.x = i * 20.0f; // (rand() % 640) / 640.0f;
        particleList[i].pos.y = -200.0f; // (rand() % 480) / 480.0f;
        particleList[i].pos.z =  0.0f;

        particleInfoList[i].dir.emplace_back(constant::Vector3D(0.0f, -200.0f, 0.0f));

        for(int j = 0; j < 30; j++)
            particleInfoList[i].dir.emplace_back(constant::Vector3D(0.0f, float(-(rand() % 400)), 0.0f));
    }

    // Square VAO Creation

    SquareVAO = new QOpenGLVertexArrayObject();
	SquareVBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	SquareEBO = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);

    instanceVBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);

    SquareVAO->create();
    SquareVBO->create();
    SquareEBO->create();

    instanceVBO->create();

    auto tileVertices = new float[20];

    ////                 x                         y                         z                      texpos x                 texpos y
	//tileVertices[0] =  -0.5f; tileVertices[1] =   0.5f; tileVertices[2] =  0.0f; tileVertices[3] =  0.0f; tileVertices[4] =  1.0f;   
	//tileVertices[5] =   0.5f; tileVertices[6] =   0.5f; tileVertices[7] =  0.0f; tileVertices[8] =  1.0f; tileVertices[9] =  1.0f;
	//tileVertices[10] = -0.5f; tileVertices[11] = -0.5f; tileVertices[12] = 0.0f; tileVertices[13] = 0.0f; tileVertices[14] = 0.0f;
	//tileVertices[15] =  0.5f; tileVertices[16] = -0.5f; tileVertices[17] = 0.0f; tileVertices[18] = 1.0f; tileVertices[19] = 0.0f;

    float width = 50.0f, height = 50.0f;

    //                 x                         y                         z                      texpos x                 texpos y
	tileVertices[0] =  0.0f ; tileVertices[1] =   0.0f  ; tileVertices[2] =  0.0f; tileVertices[3] =  0.0f; tileVertices[4] =  1.0f;   
	tileVertices[5] =  width; tileVertices[6] =   0.0f  ; tileVertices[7] =  0.0f; tileVertices[8] =  1.0f; tileVertices[9] =  1.0f;
	tileVertices[10] = 0.0f ; tileVertices[11] = -height; tileVertices[12] = 0.0f; tileVertices[13] = 0.0f; tileVertices[14] = 0.0f;
	tileVertices[15] = width; tileVertices[16] = -height; tileVertices[17] = 0.0f; tileVertices[18] = 1.0f; tileVertices[19] = 0.0f;

	unsigned int nbTileVertices = 20;

	auto tileVerticesIndice = new unsigned int[6];

	tileVerticesIndice[0] = 0; tileVerticesIndice[1] = 1; tileVerticesIndice[2] = 2;
	tileVerticesIndice[3] = 1; tileVerticesIndice[4] = 2; tileVerticesIndice[5] = 3;

	unsigned int nbOfElements = 6;

    SquareVAO->bind();

    // position attribute
    
    SquareVBO->bind();
    SquareVBO->setUsagePattern(QOpenGLBuffer::StaticDraw);
    SquareVBO->allocate(tileVertices, nbTileVertices * sizeof(float));

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    // texture coord attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    instanceVBO->bind();
    instanceVBO->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    instanceVBO->allocate(particleList, nbMaxParticle * sizeof(Particle));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    extraFunctions->glVertexAttribDivisor(2, 1);

    SquareEBO->bind();
    SquareEBO->setUsagePattern(QOpenGLBuffer::StaticDraw);
    SquareEBO->allocate(tileVerticesIndice, nbOfElements * sizeof(unsigned int));

    SquareVAO->release();

    camera = new Camera(QVector3D(0.0f, 0.0f, 3.0f));

    QObject::connect(inputHandler, SIGNAL(updatedKeyInput(Input*, double)), camera, SLOT(updateKeyboard(Input*, double)));
    QObject::connect(inputHandler, SIGNAL(updatedMouseInput(Input*, double)), camera, SLOT(updateMouse(Input*, double)));

    ticking = true;
    std::thread t (&GameWindow::tick, this);

    t.detach();
}

void GameWindow::render()
{
    static auto currentTime = QDateTime::currentMSecsSinceEpoch();
    static auto lastTime = QDateTime::currentMSecsSinceEpoch();

    static int nbFrames = 0;
    static auto lastFPSCount = currentTime;

    currentTime = QDateTime::currentMSecsSinceEpoch();

    //fps counter
    nbFrames++;
    if(currentTime - lastFPSCount >= 1000 || currentTime < lastFPSCount)
    {
        std::cout << nbFrames << std::endl;
        
        nbFrames = 0;
        lastFPSCount += 1000;

        if(currentTime < lastFPSCount)
            lastFPSCount = currentTime;
    }

    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    glClearColor(0.1f, 0.3f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    currentTime = QDateTime::currentMSecsSinceEpoch();
    inputHandler->updateInput(float(currentTime - lastTime) / 1000);

    renderUi();

    lastTime = currentTime;
}

void GameWindow::setAnimating(bool animating)
{
    m_animating = animating;

    if (animating)
        renderLater();
}

void GameWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
        event->ignore();
    else
        inputHandler->registerKeyInput((Qt::Key)event->key(), Input::InputState::KEYPRESSED);
}

void GameWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
        event->ignore();
    else
        inputHandler->registerKeyInput((Qt::Key)event->key(), Input::InputState::KEYRELEASED);
}

void GameWindow::mouseMoveEvent(QMouseEvent *event)
{
    static float lastMousePosX = event->pos().x();
	static float lastMousePosY = event->pos().y();

    mousePos = event->pos();

    QPoint mouseDelta;

    mouseDelta.setX((lastMousePosX - event->pos().x()) * xSensitivity);
    mouseDelta.setY((event->pos().y() - lastMousePosY) * ySensitivity); // reversed since y-coordinates go from bottom to top
	
    //std::cout << mouseDelta.x() << mouseDelta.y() << std::endl;

    inputHandler->registerMouseMove(event->pos(), mouseDelta);

    lastMousePosX = event->pos().x();
	lastMousePosY = event->pos().y();
}

void GameWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() != Qt::NoButton)
        inputHandler->registerMouseInput((Qt::MouseButton)event->button(), Input::InputState::MOUSEPRESS);
}

void GameWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() != Qt::NoButton)
        inputHandler->registerMouseInput((Qt::MouseButton)event->button(), Input::InputState::MOUSERELEASE);
}

void GameWindow::renderLater()
{
    requestUpdate();
}

void GameWindow::renderNow()
{
    if (!isExposed())
        return;

    bool needsInitialize = false;

    if (!m_context) {
        m_context = new QOpenGLContext(this);
        m_context->setFormat(requestedFormat());
        m_context->create();

        needsInitialize = true;
    }

    m_context->makeCurrent(this);

    if (needsInitialize) {
        initializeOpenGLFunctions();
        initialize();
    }

    render();

    m_context->swapBuffers(this);

    if (m_animating)
        renderLater();
}

bool GameWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::UpdateRequest:
        renderNow();
        return true;
    default:
        return QWindow::event(event);
    }
}

void GameWindow::exposeEvent(QExposeEvent *event)
{
    Q_UNUSED(event);

    if (isExposed())
        renderNow();
}

void GameWindow::renderUi()
{
    const qreal retinaScale = devicePixelRatio();

    QMatrix4x4 projection;
    QMatrix4x4 view;
    QMatrix4x4 model;
    QMatrix4x4 scale;

    projection.setToIdentity();
    view.setToIdentity();

    guiShaderProgram->bind();

    //glBindTexture(GL_TEXTURE_2D, baseMenu1);

    projection.setToIdentity();
    view.setToIdentity();
    scale.setToIdentity();
    //scale.scale(QVector3D(2.0f, 2.0f, 0.0f));
    scale.scale(QVector3D(1.0f / width(), 1.0f / height(), 0.0f));
    model.setToIdentity();
    view.translate(QVector3D(-1.0f, 1.0f, 0.0f));

    guiShaderProgram->setUniformValue(guiShaderProgram->uniformLocation("projection"), projection);
    guiShaderProgram->setUniformValue(guiShaderProgram->uniformLocation("view"), view);
    guiShaderProgram->setUniformValue(guiShaderProgram->uniformLocation("model"), model);
    guiShaderProgram->setUniformValue(guiShaderProgram->uniformLocation("scale"), scale);

    SquareVAO->bind();

    instanceVBO->bind();
    instanceVBO->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    instanceVBO->allocate(particleList, nbMaxParticle * sizeof(Particle));

    //glEnableVertexAttribArray(2);
    //glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    //extraFunctions->glVertexAttribDivisor(2, 1);

    extraFunctions->glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, nbMaxParticle);
}

void GameWindow::tick()
{
    auto currentTime = QDateTime::currentMSecsSinceEpoch();
    auto lastTime = QDateTime::currentMSecsSinceEpoch();

    while(ticking)
    {
        lastTime = QDateTime::currentMSecsSinceEpoch();

        for(int i = 0; i < nbMaxParticle; i++)
        {
            if(particleList[i].lifetime > 0.0f)
            {
                particleList[i].lifetime -= 0.04;
                particleInfoList[i].timeElapsed += 0.04;
                int j = std::floor(particleInfoList[i].timeElapsed) + 1;
                if(j < particleInfoList[i].dir.size())
                {
                    particleList[i].pos.y += (particleInfoList[i].dir[j].y - particleInfoList[i].dir[j - 1].y) * (0.04);
                }
                //particleList[i].pos.x -= 0.1;
                //particleList[i].pos.y = -1.0f + (rand() % 5) / 2.0f;
                //particleList[i].pos.z = 0.0f;
            }

            //if(particleList[i].lifetime <= 0.0f)
            //{
            //    particleList[i].lifetime = (rand() % 15) / 10.0f;
            //    particleList[i].pos.x =  0.0f + (i / 50.0f);
            //    particleList[i].pos.y = -2.0f + (rand() % 50) / 20.0f;
            //    particleList[i].pos.z =  0.0f;
            //}
        }


        currentTime = QDateTime::currentMSecsSinceEpoch();
        std::this_thread::sleep_for(std::chrono::milliseconds(40 - (currentTime - lastTime)));
    }
}
