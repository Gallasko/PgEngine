#include "game.h"

GameWindow::GameWindow(QWindow *parent) : QWindow(parent)
{
    setSurfaceType(QWindow::OpenGLSurface);

    mousePos = QPoint(0.0f, 0.0f);

    inputHandler = new Input();
}

GameWindow::~GameWindow()
{
    ticking = false;

    if(gameMap != nullptr)
        delete gameMap;

    if(camera != nullptr)
        delete camera;

    if(inputHandler != nullptr)
        delete inputHandler;

    if(tileLoader != nullptr)
        delete tileLoader;

    if(fontLoader != nullptr)
        delete fontLoader;

    delete m_device;
}

void GameWindow::render(QPainter *painter)
{
    Q_UNUSED(painter);
}

void GameWindow::initialize()
{
	initializeOpenGLFunctions();

    masterRenderer.initialize(m_context);
    masterRenderer.setWindowSize(width(), height());

    //glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    masterRenderer.setWindowSize(640, 480);

    masterRenderer.registerShader("default", "shader/default.vs", "shader/default.fs");
    masterRenderer.registerRederer<TextureRenderer>();

    masterRenderer.registerShader("gui", "shader/default.vs", "shader/default.fs");
    masterRenderer.registerShader("text", "shader/textrendering.vs", "shader/textrendering.fs");
    masterRenderer.registerRederer<SentenceRenderer>();

    masterRenderer.registerShader("particle", "shader/particle.vs", "shader/particle.fs");
    masterRenderer.registerRederer<ParticleRenderer>();

    masterRenderer.registerTexture("atlas", "res/tiles/TeclaEatsAtlas.png");
    masterRenderer.registerTexture("menu", "res/menu/Menu.png");
    masterRenderer.registerTexture("font", "res/font/font.png");
    masterRenderer.registerTexture("pigeon", "res/object/PigeonMockUp.png");

    camera = new Camera(QVector3D(0.0f, 0.0f, 3.0f));

    tileLoader = new TilesLoader("res/tiles/");

    fontLoader = new FontLoader("res/font/fontmap.ft");

    mapConstraint.width = 5;
    mapConstraint.height = 5;
    mapConstraint.seed = 1;
 
    mapConstraint.noiseParam = {4, 5, 50, -1, 0.4};

    gameMap = new Map(&ecs, tileLoader, mapConstraint);

    auto debugText = ecs.createEntity();
    auto debugTextC = ecs.attach<Sentence>(debugText, {{"Debug: ", constant::Vector4D(255.0f, 0.0f, 0.0f, 255.0f)}, 4.0f, fontLoader});

    debugTextC->setX(10);
    debugTextC->setY(10);

    fpsCounter = ecs.createEntity();
    auto fpsCounterC = ecs.attach<Sentence>(fpsCounter, {{"00"}, 4.0f, fontLoader});
    
    fpsCounterC->setX(10);
    fpsCounterC->setY(55);
    
    auto mousePosLabel = ecs.createEntity();
    auto mousePosLabelC = ecs.attach<Sentence>(mousePosLabel, {{"Mouse Pos: "}, 2.0f, fontLoader});
    
    mousePosLabelC->setX(10);
    mousePosLabelC->setY(100);

    mousePosText = ecs.createEntity();
    auto mousePosTextC = ecs.attach<Sentence>(mousePosText, {{"(0, 0)"}, 2.0f, fontLoader});
    
    mousePosTextC->setX(10);
    mousePosTextC->setZ(2);
    mousePosTextC->setY(125);

    gameScaleText = ecs.createEntity();
    auto gameScaleTextC = ecs.attach<Sentence>(gameScaleText, {{"Game Scale: 0"}, 2.0f, fontLoader});
    
    gameScaleTextC->setX(10);
    gameScaleTextC->setZ(2);
    gameScaleTextC->setY(150);

    screenEntity = ecs.createEntity();
    screenUi = ecs.attach<UiComponent>(screenEntity, {});
    //screenUi->setWidth(width());
    //screenUi->setHeight(height());
    screenUi->setWidth(1);
    screenUi->setHeight(1);
    screenUi->setZ(0);

    std::cout << width() << std::endl;
    std::cout << screenUi->right << std::endl;

    auto screenInput = ecs.attach<MouseInputComponent*>(screenEntity, {});
    *screenInput = new MouseInputBase<Camera>(screenUi);

    (*screenInput)->registerFunc(Camera::updateMouse, camera);

    auto screenKeyInput = ecs.attach<KeyboardInputComponent* >(screenEntity, {});
    *screenKeyInput = new KeyboardInputBase<Camera>();

    (*screenKeyInput)->registerFunc(Camera::updateKeyboard, camera);

    auto mapTemp = ecs.createEntity();
    auto mapKeyInput = ecs.attach<KeyboardInputComponent* >(screenEntity, {});
    *mapKeyInput = new KeyboardInputBase<Map>();

    //(*mapKeyInput)->registerFunc(&gameMap->switchToPathFind, gameMap);
    (*mapKeyInput)->registerFunc(Map::switchToPathFind, gameMap);

    mapClickComponent = new MouseInputBase<Map>(screenUi);
    mapClickComponent->registerFunc(Map::clicked, gameMap);

    tileSelector = new TileSelector(gameMap, tileLoader, fontLoader, screenUi);
    tileSelector->pos.z = 2;

    //Sequence tileSelectorSeq = Sequence(
    //    //Sequence::OriginPoint(pathFindingButtonTexC->pos.x, pathFindingButtonTexC->pos.y, pathFindingButtonTexC->pos.z),
    //    UiKeyPoint(screenUi->width - tileSelector->width, screenUi->height - tileSelector->height, 0.0f, 0),
    //    UiKeyPoint(screenUi->width - tileSelector->width, screenUi->height - tileSelector->height - 100.0f, 0.0f, 0),
    //    UiKeyPoint(screenUi->width - tileSelector->width - 100.0f, screenUi->height - tileSelector->height - 100.0f, 0.0f, 0),
    //    UiKeyPoint(screenUi->width - tileSelector->width - 100.0f, screenUi->height - tileSelector->height, 0.0f, 0)
    //);

    Sequence tileSelectorSeq = Sequence(
       //Sequence::OriginPoint(pathFindingButtonTexC->pos.x, pathFindingButtonTexC->pos.y, pathFindingButtonTexC->pos.z),
       UiKeyPoint(400.0f, 400.0f, 2.0f, 0),
       UiKeyPoint(500.0f, 400.0f, 2.0f, 1000),
       UiKeyPoint(500.0f, 500.0f, 2.0f, 2000),
       UiKeyPoint(400.0f, 500.0f, 2.0f, 3000)
    );

    AnimationComponent *tileSelectorAnimation = new AnimationComponent(tileSelector, tileSelectorSeq, true);
    tileSelectorAnimation->start();

    auto pathFindingButton = ecs.createEntity();
    auto pathFindingButtonTexC = ecs.attach<TextureComponent>(pathFindingButton, {64, 32, "res/menu/frame.png"});
    pathFindingButtonTexC->setX(0);
    pathFindingButtonTexC->setY(height() - 32);
    pathFindingButtonTexC->setZ(1);

    auto pathFindingButtonMouseArea = ecs.attach<MouseInputComponent* >(pathFindingButton, {});
    *pathFindingButtonMouseArea = new MouseInputBase<Map>(pathFindingButtonTexC);
    (*pathFindingButtonMouseArea)->registerFunc(Map::runPathFinding, gameMap);

    Sequence seq = Sequence(
        //Sequence::OriginPoint(pathFindingButtonTexC->pos.x, pathFindingButtonTexC->pos.y, pathFindingButtonTexC->pos.z),
        UiKeyPoint(100.0f, 100.0f, 0.0f, 0),
        UiKeyPoint(200.0f, 100.0f, 0.0f, 1000),
        UiKeyPoint(200.0f, 200.0f, 0.0f, 2000),
        UiKeyPoint(100.0f, 200.0f, 0.0f, 3000),
        UiKeyPoint(100.0f, 100.0f, 0.0f, 4000)
    );

    AnimationComponent *animation = new AnimationComponent(pathFindingButtonTexC, seq, true);
    animation->start();

    //cmpTexTest = new TextureComponent(300, 300, "res/menu/Menu2.png");

    /*
    //Particle Gen
    pComponent = new ParticleComponent();
    pComponent->instanceVBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    if(pComponent->instanceVBO->create())
        std::cout << "Created" << std::endl;
    pComponent->count = 4000;

    auto extraFunctions = masterRenderer.getExtraFunctions();

    auto tileVertices = new float[20];

    //make it so i set x and y at 1 and the scaling is done in the shader
    //                 x                         y                         z                      texpos x                 texpos y
    tileVertices[0]  = 0.0f; tileVertices[1]  =  0.0f; tileVertices[2]  = 0.0f; tileVertices[3]  = 0.0f;  tileVertices[4]  = 0.0f;   
    tileVertices[5]  = 1.0f; tileVertices[6]  =  0.0f; tileVertices[7]  = 0.0f; tileVertices[8]  = 0.25f; tileVertices[9]  = 0.0f;
    tileVertices[10] = 0.0f; tileVertices[11] = -1.0f; tileVertices[12] = 0.0f; tileVertices[13] = 0.0f;  tileVertices[14] = 1.0f;
    tileVertices[15] = 1.0f; tileVertices[16] = -1.0f; tileVertices[17] = 0.0f; tileVertices[18] = 0.25f; tileVertices[19] = 1.0f;
    //texPos x is set at 0.25f because the sprite for the pigeon is 4 frames wide

    unsigned int nbTileVertices = 20;

    auto tileVerticesIndice = new unsigned int[6];

    tileVerticesIndice[0] = 0; tileVerticesIndice[1] = 1; tileVerticesIndice[2] = 2;
    tileVerticesIndice[3] = 1; tileVerticesIndice[4] = 2; tileVerticesIndice[5] = 3;

    unsigned int nbOfElements = 6;

    pComponent->openglObject.initialize();
    pComponent->openglObject.VAO->bind();

    // position attribute

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    
    pComponent->openglObject.VBO->bind();
    pComponent->openglObject.VBO->setUsagePattern(QOpenGLBuffer::StaticDraw);
    pComponent->openglObject.VBO->allocate(tileVertices, nbTileVertices * sizeof(float));

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    // texture coord attribute
    
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    std::cout << "set instance vbo" << std::endl;

    //glBindBuffer(GL_ARRAY_BUFFER, pComponent->instanceVBO);
    //glBufferData(GL_ARRAY_BUFFER, pComponent->count * sizeof(Particle), NULL, GL_STREAM_DRAW);

    pComponent->instanceVBO->bind();
    pComponent->instanceVBO->setUsagePattern(QOpenGLBuffer::StreamDraw);
    pComponent->instanceVBO->allocate(pComponent->count * sizeof(Particle));
    //masterRenderer.getInstanceVBO()->bind();

    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)0);

    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, pos));

    //TODO check if we can send the tex vertex only once and not twice : once here and the second time in the squareVAO implementation 
    
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, texOffset));
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, scale));
    
    extraFunctions->glVertexAttribDivisor(2, 1);
    extraFunctions->glVertexAttribDivisor(3, 1);
    extraFunctions->glVertexAttribDivisor(4, 1);
    extraFunctions->glVertexAttribDivisor(5, 1);

    pComponent->openglObject.EBO->bind();
    pComponent->openglObject.EBO->setUsagePattern(QOpenGLBuffer::StaticDraw);
    pComponent->openglObject.EBO->allocate(tileVerticesIndice, nbOfElements * sizeof(unsigned int));

    pComponent->openglObject.VAO->release();
    
    pComponent->texture = masterRenderer.getTexture("pigeon");

    pComponent->particleList = new Particle[pComponent->count];
    pComponent->particleSubDataList = new ParticleSubData*[pComponent->count];

    std::vector<float> textureSeq = { 0.00f, 0.25f, 0.50f, 0.75f } ;

    for(int i = 0; i < pComponent->count; i++)
    {
        pComponent->particleList[i].lifetime = 10000.0f;
        pComponent->particleList[i].pos = constant::Vector3D((48 * i) % (width() * 2), -48.0f * ((48 * (i + 200)) / (width() * 2)), 0.0f);
        pComponent->particleList[i].texOffset = 0.25f;
        pComponent->particleList[i].scale = ((rand() % 6) + 1) * 8;

        //TODO create a texture sequence struct that store the different transition, calculate there position and hold the timing
        pComponent->particleSubDataList[i] = new ParticleMoveSubData(constant::Vector3D(0.0f, 0.0f, 0.0f), textureSeq, ((rand() % 8) + 1) * 40);
    }

    // TODO correclty define tick rate as 40ms (25 ticks each second)
    pComponent->onTick = [=]() { 
        for(int i = 0; i < pComponent->count; i++) 
        {
            ParticleMoveSubData *pMoveData = static_cast<ParticleMoveSubData*>(pComponent->particleSubDataList[i]);
            pMoveData->timeAlive += 40;
            pComponent->particleList[i].lifetime -= 40;
            pComponent->particleList[i].pos += pMoveData->velocity;
            //TODO check if change rate is 0 so that we don t divide by 0 !!
            pComponent->particleList[i].texOffset = pMoveData->textureSeq[(pMoveData->timeAlive / pMoveData->textureChangeRate) % pMoveData->textureSeq.size()];
        }};

    */

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

    try
    {
        if(screenUi->width != width())
        {
            std::cout << "Width Update" << std::endl;
            screenUi->setWidth(width());
            masterRenderer.setWindowSize(width(), height());
        }
            
        if(screenUi->height != height())
        {
            std::cout << "Height Update" << std::endl;
            screenUi->setHeight(height());
            masterRenderer.setWindowSize(width(), height());
        }
    }
    catch(const std::exception& e)
    {
        std::cout << "Resize error" << std::endl;
        std::cerr << e.what() << '\n';
    }

    masterRenderer.setCurrentTime(currentTime);

    //fps counter
    nbFrames++;
    if(currentTime - lastFPSCount >= 1000 || currentTime < lastFPSCount)
    {
        auto fpsText = fpsCounter->get<Sentence>();
        if(fpsText != nullptr)
            fpsText->setText(std::to_string(nbFrames), fontLoader);
        nbFrames = 0;
        lastFPSCount += 1000;

        if(currentTime < lastFPSCount)
            lastFPSCount = currentTime;
    }

    auto mousePosTextC = mousePosText->get<Sentence>();
    if(mousePosTextC != nullptr)
        mousePosTextC->setText("(" + std::to_string(mousePos.x()) + ", " + std::to_string(mousePos.y()) + ")", fontLoader);

    auto gameScaleTextC = gameScaleText->get<Sentence>();
    if(gameScaleTextC != nullptr)
        gameScaleTextC->setText("Game Scale: " + std::to_string(static_cast<int>(gameScale)), fontLoader);

    updateGameState(float(currentTime - lastTime) / 1000);

    renderGame();

    if(!debug)
    {
        auto mousePosTextC = mousePosText->get<Sentence>();
        if(mousePosTextC != nullptr)
            mousePosTextC->visible = true;

        renderUi();
    }
    else
    {
        auto mousePosTextC = mousePosText->get<Sentence>();
        if(mousePosTextC != nullptr)
            mousePosTextC->visible = false;
    }

    masterRenderer << tileSelector;

    //masterRenderer.render<ParticleRenderer>(pComponent);

    inputHandler->updateInput(float(currentTime - lastTime) / 1000);

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

void GameWindow::changeRandomText(Input* inputHandler, double deltaTime) 
{
    static std::vector<int> keyPressed;

    int shiftKey = inputHandler->isKeyPressed(Qt::Key_Shift) ? 0 : 32; // Number to shift from upper to lower case 
    bool ctrlKey = inputHandler->isKeyPressed(Qt::Key_Control);

    for(int i = 0x41; i <= 0x5a; i++) // QtKey range from A = 0x41 to Z = 0x5a
    {
        if(inputHandler->isKeyPressed(static_cast<Qt::Key>(i)) && std::find(keyPressed.begin(), keyPressed.end(), i) == keyPressed.end())
        {
            randomText += static_cast<char>(i + shiftKey);
            keyPressed.push_back(i);
        } 
    }

    if(inputHandler->isKeyPressed(Qt::Key_Space) && std::find(keyPressed.begin(), keyPressed.end(), static_cast<int>(Qt::Key_Space)) == keyPressed.end()) 
    {
        randomText += " ";
        keyPressed.push_back(static_cast<int>(Qt::Key_Space));
    }

    if(inputHandler->isKeyPressed(Qt::Key_Backspace) && randomText.size() > 0 && std::find(keyPressed.begin(), keyPressed.end(), static_cast<int>(Qt::Key_Backspace)) == keyPressed.end()) 
    {
        if(ctrlKey)
            randomText = "";
        else
            randomText.pop_back();

        keyPressed.push_back(static_cast<int>(Qt::Key_Backspace));
    }

    for(auto key : keyPressed)
        if(!inputHandler->isKeyPressed(static_cast<Qt::Key>(key)))
            keyPressed.erase(std::find(keyPressed.begin(), keyPressed.end(), key));

    auto text = userText->get<Sentence>();
    if(text != nullptr)
        text->setText(randomText, fontLoader); 
}

void GameWindow::payTeclaFlooz(Input *inputHandler, double deltaTime)
{
    static bool pressed = false;

    if(inputHandler->isButtonPressed(Qt::LeftButton) && !pressed)
    {
        gold -= 10;
        pressed = true;
    }

    if(!inputHandler->isButtonPressed(Qt::LeftButton))
        pressed = false;
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
    
    try
    {
        render();
    }
    catch(const std::exception& e)
    {
        std::cout << "Render error" << std::endl;
        std::cerr << e.what() << '\n';
    }
    
    m_context->swapBuffers(this);
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

void GameWindow::updateGameState(double deltaTime)
{
    int highestZ = -1;

    // Take the Highest Z under the mouse and make only those element clickable  
    for(auto mouseArea : ecs.view<MouseInputComponent*>())
        if(mouseArea->inBound(mousePos.x(), mousePos.y()) && *mouseArea->enable)
            if (mouseArea->pos->z > highestZ)
                highestZ = mouseArea->pos->z;

    if(mousePos.x() > tileSelector->pos.x / static_cast<int>(tileSelector->scale) && mousePos.x() < (tileSelector->pos.x + tileSelector->width) / static_cast<int>(tileSelector->scale) && mousePos.y() < (tileSelector->pos.y + tileSelector->height) / static_cast<int>(tileSelector->scale) && mousePos.y() > tileSelector->pos.y / static_cast<int>(tileSelector->scale) && tileSelector->isVisible())
        if (tileSelector->pos.z > highestZ)
            highestZ = tileSelector->pos.z;

    for(auto mouseArea : ecs.view<MouseInputComponent*>())
    {
        if(mouseArea->inBound(mousePos.x(), mousePos.y()) && *mouseArea->enable && mouseArea->pos->z == highestZ)
        {
            //std::cout << "Mouse Hovering: " << *mouseArea.x << ", " << *mouseArea.y << ", " << *mouseArea.width << ", " << *mouseArea.height << std::endl;
            mouseArea->call(inputHandler, deltaTime);
        }
    }

    if(tileSelector->pos.z == highestZ)
        tileSelector->mouseInput(inputHandler, deltaTime);

    //TODO make the map responsive to Z index 
    if(highestZ <= 0)
        mapClickComponent->call(inputHandler, deltaTime, width(), height(), gameScale, camera);

    for(auto keyArea : ecs.view<KeyboardInputComponent*>())
    {
        keyArea->call(inputHandler, deltaTime);
    }
}

void GameWindow::renderGame()
{
    static unsigned long long nbRenderGameFrame = 0;

    const qreal retinaScale = devicePixelRatio();

    QMatrix4x4 projection;
    QMatrix4x4 view;
    QMatrix4x4 model;
    QMatrix4x4 scale;

    auto defaultShaderProgram = masterRenderer.getShader("default");

    defaultShaderProgram->bind();

    projection.setToIdentity();
    projection.ortho(-1.0f, 1.0f, -1.0f, 1.0f, -100.0f, 100.0f); // Fix the zoom

    view = camera->GetViewMatrix();
    scale.setToIdentity();
    scale.scale(QVector3D(gameScale / width(), gameScale / height(), 0.0f));

    auto baseTileTexture1 = masterRenderer.getTexture("atlas");

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
                    tileLoader->getTile("Selected Tile")->getMesh()->bind();
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                    tileSelected = true;
                }
            }
        }
    }

    defaultShaderProgram->release();

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
    scale.setToIdentity();
    scale.scale(QVector3D(2.0f / width(), 2.0f / height(), 0.0f));

    // Text rendering

    auto defaultShaderProgram = masterRenderer.getShader("default");

    defaultShaderProgram->bind();

    defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("projection"), projection);
    defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("model"), model);
    defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("scale"), scale);

    //gl scissor for list views 
    //glEnable(GL_SCISSOR_TEST);
    //glScissor(300, 200, 200, 500);

    for(auto& texture : ecs.view<TextureComponent>())
    {
        if(texture.visible)
        {
            if(texture.initialised == false)
                texture.generateMesh();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture.texture);

            view.setToIdentity();
            view.translate(QVector3D(-1.0f + 2.0f * (float)(texture.pos.x) / width(), 1.0f + 2.0f * (float)( -texture.pos.y) / height(), 0.0f));

            defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("view"), view);

            texture.VAO->bind();
            glDrawElements(GL_TRIANGLES, texture.modelInfo.nbIndices, GL_UNSIGNED_INT, 0);
        }
    }

    //glDisable(GL_SCISSOR_TEST);

    defaultShaderProgram->release();

    auto textShaderProgram = masterRenderer.getShader("text");
    
    textShaderProgram->bind();

    scale.setToIdentity();
    scale.scale(QVector3D(2.0f / width(), 2.0f / height(), 0.0f));

    glActiveTexture(GL_TEXTURE0);
    auto fontTexture = masterRenderer.getTexture("font");
    glBindTexture(GL_TEXTURE_2D, fontTexture);

    textShaderProgram->setUniformValue(textShaderProgram->uniformLocation("projection"), projection);
    textShaderProgram->setUniformValue(textShaderProgram->uniformLocation("model"), model);
    textShaderProgram->setUniformValue(textShaderProgram->uniformLocation("scale"), scale);

    textShaderProgram->setUniformValue(textShaderProgram->uniformLocation("time"), static_cast<int>(currentTime % 314159));

    for(auto& sentence : ecs.view<Sentence>()) //TODO set a note about how auto& is important to pass by ref and not create a copy which is costy 
    {
        if(sentence.visible)
        {
            if(sentence.initialised == false)
                sentence.generateMesh();

            view.setToIdentity();
            view.translate(QVector3D(-1.0f + 2.0 * (float)(sentence.pos.x) / width(), 1.0f + 2.0 * (float)( -sentence.pos.y) / height(), 0.0f));
            textShaderProgram->setUniformValue(textShaderProgram->uniformLocation("view"), view);

            sentence.VAO->bind();
            glDrawElements(GL_TRIANGLES, sentence.modelInfo.nbIndices, GL_UNSIGNED_INT, 0);
        }
    }

    textShaderProgram->release();
}

//TODO make a tick object that take tick function and run in background when you start up the engine
void GameWindow::tick()
{
    unsigned int tickTime = 0;

    auto currentTickTime = QDateTime::currentMSecsSinceEpoch();
    auto lastTickTime = QDateTime::currentMSecsSinceEpoch();

    while(ticking)
    {
        lastTickTime = QDateTime::currentMSecsSinceEpoch();

        //pComponent->onTick();

        gold += 1;
        //auto goldTextC = goldText->get<Sentence>();
        //if(goldTextC != nullptr)
        //    goldTextC->setText(std::to_string(gold) + " TeclaFlooz", fontLoader);
        //else
        //    std::cout << " Gold Text error " << std::endl;

        tickTime++;

        //std::cout << "animation running: "  << AnimationComponent::runningQueue.size() - 1 << std::endl;

        for(int i = AnimationComponent::runningQueue.size() - 1; i >= 0; i--) 
        {
            AnimationComponent::runningQueue[i]->tick(40); // tickRate
            
            if(!AnimationComponent::runningQueue[i]->isRunning())
                AnimationComponent::runningQueue.erase(AnimationComponent::runningQueue.begin() + i);
        }

        currentTickTime = QDateTime::currentMSecsSinceEpoch();
        std::this_thread::sleep_for(std::chrono::milliseconds(40 - (currentTickTime - lastTickTime)));
    }
}
