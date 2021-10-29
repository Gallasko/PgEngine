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

    auto screenEntity = ecs.createEntity();
    screenUi = ecs.attach<UiComponent>(screenEntity, {});
    screenUi->width = 1;
    screenUi->height = 1;
    screenUi->setZ(0);

    auto configPanel = ecs.createEntity();
    auto configPanelC = ecs.attach<TextureComponent>(configPanel, {300, 1, "res/menu/NavyBlueTexture.png"});

    configPanelC->setTopAnchor(screenUi->top);
    configPanelC->setLeftAnchor(screenUi->left);
    configPanelC->setBottomAnchor(screenUi->bottom);

    auto optionTab = ecs.createEntity();
    auto optionTabC = ecs.attach<TextureComponent>(optionTab, {300, 1, "res/menu/NavyBlueTexture.png"});

    optionTabC->setTopAnchor(screenUi->top);
    optionTabC->setRightAnchor(screenUi->right);
    optionTabC->setBottomAnchor(screenUi->bottom);

    auto optionTabName = ecs.createEntity();
    auto optionTabNameC = ecs.attach<Sentence>(optionTabName, {{"Option Tab"}, 2.0f, fontLoader});

    optionTabNameC->setTopAnchor(optionTabC->top);
    optionTabNameC->setLeftAnchor(optionTabC->left);

    auto scene = ecs.createEntity();
    sceneUi = ecs.attach<UiComponent>(scene, {});
    sceneUi->setZ(1);

    sceneUi->setTopAnchor(screenUi->top);
    sceneUi->setLeftAnchor(configPanelC->right);
    sceneUi->setBottomAnchor(screenUi->bottom);
    sceneUi->setRightAnchor(optionTabC->left); // TODO this should be the anchored to optionTab.left

    auto sceneMouseArea = ecs.attach<MouseInputComponent* >(scene, {});
    *sceneMouseArea = new MouseInputBase<GameWindow>(sceneUi);
    //(*sceneMouseArea)->registerFunc(GameWindow::sceneModification, this);
    (*sceneMouseArea)->registerFunc(GameWindow::sceneModification, this);
    //(*sceneMouseArea)->registerFunc( [](Input*, double) { std::cout << "In here" << std::endl; } );

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

    if(screenUi->width != width())
    {
        screenUi->width = width();
        masterRenderer.setWindowSize(width(), height());
    }
        
    if(screenUi->height != height())
    {
        screenUi->height = height();
        masterRenderer.setWindowSize(width(), height());
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

    updateGameState(float(currentTime - lastTime) / 1000);

    renderUi();

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
    {
        std::cout << "KeyPressed" << std::endl;
        inputHandler->registerKeyInput((Qt::Key)event->key(), Input::InputState::KEYPRESSED);
    }
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

    if(event->button() != Qt::NoButton) //TODO: check why i can t grab a button even
    {
        std::cout << "Button Pressed" << std::endl;
        inputHandler->registerMouseInput((Qt::MouseButton)event->button(), Input::InputState::MOUSEPRESS);
    }
}

void GameWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() != Qt::NoButton)
        inputHandler->registerMouseInput((Qt::MouseButton)event->button(), Input::InputState::MOUSERELEASE);
}

void GameWindow::wheelEvent(QWheelEvent *event)
{
}

void GameWindow::sceneModification(Input* inputHandler, double deltaTime...)
{
    static bool pressed = false;
    static TextureComponent* component = nullptr;

    if(inputHandler->isButtonPressed(Qt::LeftButton))
    {
        static auto startingMousePos = inputHandler->getMousePos(); 
        const auto mousePos = inputHandler->getMousePos();

        if(!pressed)
        {
            startingMousePos = mousePos;
            auto entity = ecs.createEntity();
            component = ecs.attach<TextureComponent>(entity, {1, 1, "res/menu/NavyBlueTexture.png"});

            component->pos.y = mousePos.y();
            component->pos.x = mousePos.x();

            entityTable.push_back(entity);
            pressed = true;

            auto entityName = ecs.createEntity();
            auto entityNameC = ecs.attach<Sentence>(entityName, {{"Entity " + std::to_string(entityNameList.size())}, 2.0f, fontLoader}); 

            if(entityNameList.size() == 0)
            {
                auto mousePosTextC = mousePosText->get<Sentence>();
                if(mousePosTextC != nullptr)
                {
                    entityNameC->setTopAnchor(mousePosTextC->bottom);
                    entityNameC->setLeftAnchor(mousePosTextC->left);
                }
            }
            else
            {
                auto lastEntity = entityNameList.back();
                auto lastEntityNameC = lastEntity->get<Sentence>();
                if(lastEntityNameC != nullptr)
                {
                    entityNameC->setTopAnchor(lastEntityNameC->bottom);
                    entityNameC->setLeftAnchor(lastEntityNameC->left);
                }
            }

            entityNameList.emplace_back(entityName);

            auto entityNameMouseArea = new MouseInputBase<GameWindow>(entityNameC);
            entityNameMouseArea->registerFunc(GameWindow::openConfiguration, this);

            entityNameMouseAreaList.emplace_back(entityNameMouseArea);
        }

        component->width = mousePos.x() - startingMousePos.x();
        component->height = mousePos.y() - startingMousePos.y();
    }
    else
    {
        pressed = false;
    }
}

void GameWindow::openConfiguration(Input* inputHandler, double deltaTime...)
{
    va_list args;
    va_start(args, deltaTime); 
    auto entityId = va_arg(args, int);
    va_end(args);

    std::cout << entityId << std::endl;
}


void GameWindow::changeRandomText(Input* inputHandler, double deltaTime...) 
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

    // text = userText->get<Sentence>();
    //if(text != nullptr)
    //    text->setText(randomText, fontLoader); 

    std::cout << randomText << std::endl;
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
    //static auto lastMousePos = mousePos; // TODO benchmark this and maybe push this into the official release

    //if(lastMousePos == mousePos)
    //    return;

    int highestZ = -1;

    // Take the Highest Z under the mouse and make only those element clickable  
    for(const auto& mouseArea : ecs.view<MouseInputComponent*>())
        if(mouseArea->inBound(mousePos.x(), mousePos.y()) && *mouseArea->enable)
            if (mouseArea->pos->z > highestZ)
                highestZ = mouseArea->pos->z;

    for(const auto& mouseArea : ecs.view<MouseInputComponent*>())
        if(mouseArea->inBound(mousePos.x(), mousePos.y()) && *mouseArea->enable && mouseArea->pos->z == highestZ)
            mouseArea->call(inputHandler, deltaTime);

    for(const auto& keyArea : ecs.view<KeyboardInputComponent*>())
        keyArea->call(inputHandler, deltaTime);

    //for(const auto& mouseArea : entityNameMouseAreaList)
    for(int i = 0; i < entityNameMouseAreaList.size(); i++)
        if(entityNameMouseAreaList[i]->inBound(mousePos.x(), mousePos.y()) && *entityNameMouseAreaList[i]->enable)
            entityNameMouseAreaList[i]->call(inputHandler, deltaTime, i);

    //lastMousePos = mousePos;
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
            //TODO texture.generateMesh already check if in need to initialize itself
            //if(texture.initialised == false) // TODO need to set initialised to false when an anchor is modified and width and height are changed
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

        tickTime++;

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
