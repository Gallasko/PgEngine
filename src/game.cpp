#include "game.h"

GameWindow::GameWindow(QWindow *parent) : QWindow(parent)
{
    setSurfaceType(QWindow::OpenGLSurface);

    mousePos = QPoint(0.0f, 0.0f);

    inputHandler = new Input();
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

    //glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	defaultShaderProgram = new QOpenGLShaderProgram(this);
    defaultShaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, "shader/default.vs");
    /*
    defaultShaderProgram->addCacheableShaderFromSourceCode(QOpenGLShader::Vertex,
                                                            "attribute mediump vec3 aPos;"
                                                            "attribute mediump vec2 aTexCoord;"
                                                            "varying mediump vec2 TexCoord;"
                                                            "uniform mediump mat4 model;"
                                                            "uniform mediump mat4 scale;"
                                                            "uniform mediump mat4 view;"
                                                            "uniform mediump mat4 projection;"
                                                            "void main()"
                                                            "{"
                                                            "    gl_Position =  projection * view * scale * model * vec4(aPos, 1.0f);"
                                                            "    TexCoord = vec2(aTexCoord.x, aTexCoord.y);"
                                                            "}");
    */

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

    // Square VAO Creation

    SquareVAO = new QOpenGLVertexArrayObject();
	SquareVBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	SquareEBO = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);

    SquareVAO->create();
    SquareVBO->create();
    SquareEBO->create();

    auto tileVertices = new float[20];

    //                 x                         y                         z                      texpos x                 texpos y
	tileVertices[0] =  -0.5f; tileVertices[1] =   0.5f; tileVertices[2] =  0.0f; tileVertices[3] =  0.0f; tileVertices[4] =  1.0f;   
	tileVertices[5] =   0.5f; tileVertices[6] =   0.5f; tileVertices[7] =  0.0f; tileVertices[8] =  1.0f; tileVertices[9] =  1.0f;
	tileVertices[10] = -0.5f; tileVertices[11] = -0.5f; tileVertices[12] = 0.0f; tileVertices[13] = 0.0f; tileVertices[14] = 0.0f;
	tileVertices[15] =  0.5f; tileVertices[16] = -0.5f; tileVertices[17] = 0.0f; tileVertices[18] = 1.0f; tileVertices[19] = 0.0f;

	unsigned int nbTileVertices = 20;

	auto tileVerticesIndice = new unsigned int[6];

	tileVerticesIndice[0] = 0; tileVerticesIndice[1] = 1; tileVerticesIndice[2] = 2;
	tileVerticesIndice[3] = 1; tileVerticesIndice[4] = 2; tileVerticesIndice[5] = 3;

	unsigned int nbOfElements = 6;

    SquareVAO->bind();

    // position attribute
    
    SquareVBO->bind();
    SquareVBO->setUsagePattern(QOpenGLBuffer::StreamDraw);
    SquareVBO->allocate(tileVertices, nbTileVertices * sizeof(float));

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    // texture coord attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    SquareEBO->bind();
    SquareEBO->setUsagePattern(QOpenGLBuffer::StreamDraw);
    SquareEBO->allocate(tileVerticesIndice, nbOfElements * sizeof(unsigned int));

    SquareVAO->release();

    camera = new Camera(QVector3D(0.0f, 0.0f, 3.0f));

    tileLoader = new TilesLoader("res/tiles/");

    fontLoader = new FontLoader("res/font/fontmap.ft");

    mapConstraint.width = 15;
    mapConstraint.height = 15;
    mapConstraint.seed = 1;
 
    mapConstraint.noiseParam = {4, 5, 50, -1, 0.4};

    gameMap = new Map(&ecs, tileLoader, mapConstraint);

    auto debugText = ecs.createEntity();
    auto debugTextC = ecs.attach<Sentence>(debugText, {{"Debug: ", constant::Vector4D(255.0f, 0.0f, 0.0f, 255.0f), constant::Vector4D(0.0f, 0.0f, 0.0f, 255.0f)}, 8.0f, fontLoader});
    
    debugTextC->setX(10);
    debugTextC->setY(10);

    fpsCounter = ecs.createEntity();
    auto fpsCounterC = ecs.attach<Sentence>(fpsCounter, {{"00"}, 8.0f, fontLoader});
    
    fpsCounterC->setX(10);
    fpsCounterC->setTopAnchor(debugTextC);
    fpsCounterC->setTopMargin(10);

    auto fpsText = ecs.createEntity();
    auto fpsTextC = ecs.attach<Sentence>(fpsText, {{"Fps", constant::Vector4D(75.0f, 0.0f, 130.0f, 255.0f), constant::Vector4D(0.0f, 0.0f, 0.0f, 255.0f)}, 8.0f, fontLoader});
    
    fpsTextC->setX(10);
    fpsTextC->setTopAnchor(debugTextC);
    fpsTextC->setTopMargin(10);
    fpsTextC->setLeftAnchor(fpsCounterC);
    fpsTextC->setLeftMargin(10);

    auto text = ecs.createEntity();
    auto textC = ecs.attach<Sentence>(text, {{"ABCDEFGHIJKLMN", constant::Vector4D(0.0f, 0.0f, 128.0f, 255.0f), constant::Vector4D(255.0f, 255.0f, 255.0f, 190.0f), constant::Vector4D(0.0f, 0.0f, 0.0f, 0.0f)}, 2.2f, fontLoader});
    //auto textC = ecs.attach<Sentence>(text, {{"ABCDEFGHIJKLMN", constant::Vector4D(75.0f, 0.0f, 130.0f, 255.0f), constant::Vector4D(0.0f, 0.0f, 0.0f, 255.0f), constant::Vector4D(0.0f, 0.0f, 0.0f, 0.0f)}, 4.0f, fontLoader});

    textC->setX(10);
    textC->setTopAnchor(fpsTextC);
    textC->setTopMargin(10);

    auto text2 = ecs.createEntity();
    //auto text2C = ecs.attach<Sentence>(text2, {{"OPQRSTUVWXYZ"}, 4.0f, fontLoader});    
    auto text2C = ecs.attach<Sentence>(text2, {{"OPQRSTUVWXYZ", constant::Vector4D(255.0f, 255.0f, 255.0f, 255.0f), constant::Vector4D(0.0f, 0.0f, 0.0f, 0.0f), constant::Vector4D(0.0f, 0.0f, 0.0f, 0.0f)}, 4.0f, fontLoader});
    
    text2C->setX(10);
    text2C->setTopAnchor(textC);
    text2C->setTopMargin(10);

    auto text3 = ecs.createEntity();
    auto text3C = ecs.attach<Sentence>(text3, {{"abcdefghijklmn"}, 2.2f, fontLoader});
    
    text3C->setX(10);
    text3C->setTopAnchor(text2C);
    text3C->setTopMargin(10);

    auto text4 = ecs.createEntity();
    auto text4C = ecs.attach<Sentence>(text4, {{"opqrstuvwxyz"}, 1.5f, fontLoader});
    
    text4C->setX(10);
    text4C->setTopAnchor(text3C);
    text4C->setTopMargin(10);

    auto mousePosLabel = ecs.createEntity();
    auto mousePosLabelC = ecs.attach<Sentence>(mousePosLabel, {{"Mouse Pos: "}, 8.1f, fontLoader});
    
    mousePosLabelC->setX(10);
    mousePosLabelC->setTopAnchor(text4C);
    mousePosLabelC->setTopMargin(10);

    mousePosText = ecs.createEntity();
    auto mousePosTextC = ecs.attach<Sentence>(mousePosText, {{"(0, 0)"}, 9.5f, fontLoader});
    
    mousePosTextC->setX(10);
    mousePosTextC->setTopAnchor(mousePosLabelC);
    mousePosTextC->setTopMargin(10);

    auto tilePosLabel = ecs.createEntity();
    auto tilePosLabelC = ecs.attach<Sentence>(tilePosLabel, {{"Tile Pos: "}, 4.0f, fontLoader});
    
    tilePosLabelC->setX(10);
    tilePosLabelC->setTopAnchor(mousePosTextC);
    tilePosLabelC->setTopMargin(10);

    tilePosText = ecs.createEntity();
    auto tilePosTextC = ecs.attach<Sentence>(tilePosText, {{"(0, 0)"}, 4.0f, fontLoader});
    
    tilePosTextC->setX(10);
    tilePosTextC->setTopAnchor(tilePosLabelC);
    tilePosTextC->setTopMargin(10);

    tileType = ecs.createEntity();
    auto tileTypeC = ecs.attach<Sentence>(tileType, {{"None"}, 4.0f, fontLoader});
    
    tileTypeC->setX(10);
    tileTypeC->setTopAnchor(tilePosTextC);
    tileTypeC->setTopMargin(10);

    goldText = ecs.createEntity();
    auto goldTextC = ecs.attach<Sentence>(goldText, {{"0 TeclaFlooz"}, 4.0f, fontLoader});

    goldTextC->setX(10);
    goldTextC->setTopAnchor(tileTypeC);
    goldTextC->setTopMargin(10);

    nbRenderedGameFrameText = ecs.createEntity();
    auto nbRenderedGameFrameTextC = ecs.attach<Sentence>(nbRenderedGameFrameText, {{"Nb Rendered Game Frame: 0"}, 4.0f, fontLoader});

    nbRenderedGameFrameTextC->setX(10);
    nbRenderedGameFrameTextC->setTopAnchor(goldTextC);
    nbRenderedGameFrameTextC->setTopMargin(10);

    currentSeedText = ecs.createEntity();
    auto currentSeedTextC = ecs.attach<Sentence>(currentSeedText, {{"Current Seed: 0"}, 4.0f, fontLoader});

    currentSeedTextC->setX(10);
    currentSeedTextC->setTopAnchor(nbRenderedGameFrameTextC);
    currentSeedTextC->setTopMargin(10);

    QObject::connect(inputHandler, SIGNAL(updatedKeyInput(Input*, double)), camera, SLOT(updateKeyboard(Input*, double)));
    QObject::connect(inputHandler, SIGNAL(updatedMouseInput(Input*, double)), camera, SLOT(updateMouse(Input*, double)));

    ticking = true;
    std::thread t (&GameWindow::tick, this);

    t.detach();
}

void GameWindow::render()
{
    currentTime = QDateTime::currentMSecsSinceEpoch();
    static auto lastTime = QDateTime::currentMSecsSinceEpoch();

    static int nbFrames = 0;
    static auto lastFPSCount = currentTime;

    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    glClearColor(0.1f, 0.3f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    currentTime = QDateTime::currentMSecsSinceEpoch();
    inputHandler->updateInput(float(currentTime - lastTime) / 1000);

    //fps counter
    nbFrames++;
    if(currentTime - lastFPSCount >= 1000 || currentTime < lastFPSCount)
    {
        auto fpsText = fpsCounter->get<Sentence>();
        if(fpsText != nullptr)
            fpsText->setText(std::to_string(nbFrames), fontLoader);
        else
            std::cout << "Fps Text Error" << std::endl;
        nbFrames = 0;
        lastFPSCount += 1000;

        if(currentTime < lastFPSCount)
            lastFPSCount = currentTime;
    }

    auto mousePosTextC = mousePosText->get<Sentence>();
    if(mousePosTextC != nullptr)
        mousePosTextC->setText("(" + std::to_string(mousePos.x()) + ", " + std::to_string(mousePos.y()) + ")", fontLoader);
    else
        std::cout << " Mouse Pos Text error" << std::endl;

    updateGameState();

    renderGame();

    if(debug)
        renderUi();

    auto nbRenderedGameFrameTextC = nbRenderedGameFrameText->get<Sentence>();
    if(nbRenderedGameFrameTextC != nullptr)
        nbRenderedGameFrameTextC->setText("Render Time: " + std::to_string(currentTime - lastTime) + "ms", fontLoader);
    else
        std::cout << "Nb Rendered Game Frame Text Error" << std::endl;

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
    {
        inputHandler->registerKeyInput((Qt::Key)event->key(), Input::InputState::KEYPRESSED);
        if(event->key() == Qt::Key_Plus)
        {
            mapConstraint.width += 5;
            mapConstraint.height += 5;
        }
        else if(event->key() == Qt::Key_S)
        {
            if(mapConstraint.width > 5)
            {
                mapConstraint.width -= 5;
                mapConstraint.height -= 5;
            }
        }
    }

    if(inputHandler->isKeyPressed(Qt::Key_Control) && inputHandler->isKeyPressed(Qt::Key_3) && !debugSwitched)
    {
        debug = !debug;
        debugSwitched = true;
    }
}

void GameWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
        event->ignore();
    else
        inputHandler->registerKeyInput((Qt::Key)event->key(), Input::InputState::KEYRELEASED);

    if(inputHandler->isKeyReleased(Qt::Key_Control) || inputHandler->isKeyReleased(Qt::Key_3))
        debugSwitched = false;
}

void GameWindow::mouseMoveEvent(QMouseEvent *event)
{
    QPoint mouseDelta;

    mouseDelta.setX((mousePos.x() - event->pos().x()) * xSensitivity);
    mouseDelta.setY((event->pos().y() - mousePos.y()) * ySensitivity); // reversed since y-coordinates go from bottom to top

    inputHandler->registerMouseMove(event->pos(), mouseDelta);

    mousePos = event->pos();
}

void GameWindow::mousePressEvent(QMouseEvent *event)
{
    mousePos = event->pos();

    if(event->button() != Qt::NoButton)
        inputHandler->registerMouseInput((Qt::MouseButton)event->button(), Input::InputState::MOUSEPRESS);
}

void GameWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() != Qt::NoButton)
        inputHandler->registerMouseInput((Qt::MouseButton)event->button(), Input::InputState::MOUSERELEASE);
}

void GameWindow::wheelEvent(QWheelEvent *event)
{
    gameScale += (event->angleDelta().y() / 120) * 5.0f;
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

void GameWindow::updateGameState()
{

}

void GameWindow::renderGame()
{
    static unsigned long long nbRenderGameFrame = 0;
/*
    auto nbRenderedGameFrameTextC = nbRenderedGameFrameText->get<Sentence>();
    if(nbRenderedGameFrameTextC != nullptr)
        nbRenderedGameFrameTextC->setText("Nb Rendered Game Frame: " + std::to_string(nbRenderGameFrame), fontLoader);
    else
        std::cout << "Nb Rendered Game Frame Text Error" << std::endl;
*/
    const qreal retinaScale = devicePixelRatio();

    QMatrix4x4 projection;
    QMatrix4x4 view;
    QMatrix4x4 model;
    QMatrix4x4 scale;

    defaultShaderProgram->bind();

    projection.setToIdentity();
    projection.ortho(-1.0f, 1.0f, -1.0f, 1.0f, -100.0f, 100.0f); // Fix the zoom

    view = camera->GetViewMatrix();
    scale.setToIdentity();
    scale.scale(QVector3D(gameScale / width(), gameScale / height(), 0.0f));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, baseTileTexture1);

    defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("texture1"), 0);

    // [ TODO ] Fix the camera deplacement

    float selectedTileX = ((float)(mousePos.x() - width() / 2.0f )) / (gameScale / 2.0f) + camera->Position.x() * width() / gameScale;
    float selectedTileY = ((float)(height() / 2.0f - mousePos.y())) / (gameScale / 4.0f) + camera->Position.y() * height() / gameScale * 2;

    bool tileSelected = false;

    if(gameMap != nullptr)
    {
        
        auto tileMap = gameMap->getTileMap();

        for(int x = gameMap->getWidth() - 1; x >= 0; x--)
        {
            for(int y = gameMap->getHeight() - 1; y >= 0; y--)
            {
                auto tile = tileMap[x][y];

                model.setToIdentity();
                model.translate(QVector3D((tile->x - tile->y) / 2.0f, (tile->x + tile->y) / 4.0f, 0.0f));

                defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("projection"), projection);
                defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("view"), view);
                defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("model"), model);
                defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("scale"), scale);
                
                tile->tileId->getMesh()->bind();
                glDrawElements(GL_TRIANGLES, 6*6, GL_UNSIGNED_INT, 0);

                if(x == std::floor(selectedTileX + selectedTileY + 1) &&  y == std::floor(selectedTileY - selectedTileX + 1))
                {
                    auto tilePosTextC = tilePosText->get<Sentence>();
                    if(tilePosTextC != nullptr)
                        tilePosTextC->setText("(" + std::to_string(x) + ", " + std::to_string(y) + ") NValue: " + std::to_string(tile->nValue), fontLoader);
                    else
                        std::cout << " Tile Pos error " << std::endl;

                    auto tileTypeC = tileType->get<Sentence>();
                    if(tileTypeC != nullptr)
                        tileTypeC->setText(tile->tileId->getName(), fontLoader);
                    else
                        std::cout << " Tile Type error " << std::endl;

                    tileLoader->getTile("Selected Tile")->getMesh()->bind();
                    glDrawElements(GL_TRIANGLES, 6*6, GL_UNSIGNED_INT, 0);

                    tileSelected = true;
                }
            }
        }
        
       /*
        model.setToIdentity();
        model.translate(QVector3D((0) / 2.0f, (0) / 4.0f, 0.0f));

        defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("projection"), projection);
        defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("view"), view);
        defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("model"), model);
        defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("scale"), scale);
        
        gameMap->getMesh()->bind();
        glDrawElements(GL_TRIANGLES, 6*6, GL_UNSIGNED_INT, 0);
        */
    }

    defaultShaderProgram->release();

    if(nbRenderGameFrame % 250 == 0)
    {
        delete gameMap;

        mapConstraint.seed = nbRenderGameFrame / 250;

        gameMap = new Map(&ecs, tileLoader, mapConstraint);

        auto currentSeedTextC = currentSeedText->get<Sentence>();
        if(currentSeedTextC != nullptr)
            currentSeedTextC->setText("Current Seed: " + std::to_string(mapConstraint.seed), fontLoader);
        else
            std::cout << " Current Seed Text error" << std::endl;
    }

    if(!tileSelected)
    {
        auto tilePosTextC = tilePosText->get<Sentence>();
        if(tilePosTextC != nullptr)
            tilePosTextC->setText(std::string("None Pos"), fontLoader);
        else
            std::cout << " Tile Pos error " << std::endl;

        auto tileTypeC = tileType->get<Sentence>();
        if(tileTypeC != nullptr)
            tileTypeC->setText(std::string("None"), fontLoader);
        else
            std::cout << " Tile Type error " << std::endl;
    }

    nbRenderGameFrame++;
}

void GameWindow::renderUi()
{
    const qreal retinaScale = devicePixelRatio();

    QMatrix4x4 projection;
    QMatrix4x4 view;
    QMatrix4x4 model;
    QMatrix4x4 scale;

    projection.setToIdentity();
    model.setToIdentity();

    // Text rendering

    textShaderProgram->bind();

    glBindTexture(GL_TEXTURE_2D, fontTexture);

    textShaderProgram->setUniformValue(textShaderProgram->uniformLocation("projection"), projection);
    textShaderProgram->setUniformValue(textShaderProgram->uniformLocation("model"), model);

    textShaderProgram->setUniformValue(textShaderProgram->uniformLocation("time"), static_cast<int>(currentTime % 314159));

    for(auto sentence : ecs.view<Sentence>())
    {
        if(sentence.visible)
        {
            if(sentence.initialised == false)
                sentence.generateMesh();

            scale.setToIdentity();
            scale.scale(QVector3D(1.0f / width(), 1.0f / height(), 0.0f));

            view.setToIdentity();
            view.translate(QVector3D(-1.0f + (float)(sentence.x) / width(), 1.0f + (float)( -sentence.y) / height(), 0.0f));

            textShaderProgram->setUniformValue(textShaderProgram->uniformLocation("view"), view);
            textShaderProgram->setUniformValue(textShaderProgram->uniformLocation("scale"), scale);

            sentence.VAO->bind();
            glDrawElements(GL_TRIANGLES, sentence.modelInfo.nbIndices * 6, GL_UNSIGNED_INT, 0);
        }
    }

    textShaderProgram->release();
}

void GameWindow::tick()
{
    static unsigned int tickTime = 0;

    auto currentTickTime = QDateTime::currentMSecsSinceEpoch();
    auto lastTickTime = QDateTime::currentMSecsSinceEpoch();

    while(ticking)
    {
        lastTickTime = QDateTime::currentMSecsSinceEpoch();

        gold += 1;
        auto goldTextC = goldText->get<Sentence>();
        if(goldTextC != nullptr)
            goldTextC->setText(std::to_string(gold) + " TeclaFlooz", fontLoader);
        else
            std::cout << " Gold Text error " << std::endl;

        tickTime++;

        currentTickTime = QDateTime::currentMSecsSinceEpoch();
        std::this_thread::sleep_for(std::chrono::milliseconds(40 - (currentTickTime - lastTickTime)));
    }

    tickTime = 0;
}
