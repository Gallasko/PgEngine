#include "app.h"

// Todo remove
#include <QDateTime>

#include <chrono>

#include "Engine/logger.h"
#include "Engine/serialization.h" 
#include "Engine/configuration.h"

#include "Engine/Scene/sceneloader.h"

#include "Engine/ECS/loggersystem.h"

#include "UI/button.h"
#include "UI/texture.h"
#include "UI/textinput.h"

#include "Editor/Gui/contextmenu.h"
#include "Editor/Gui/optiontab.h"

// TODO create a find function in ECS

namespace
{
    static constexpr char const * DOM = "Editor window";

    struct SceneElement
    {
        SceneElement(int id, UiComponent *component, Button *mouseArea) : id(id), component(component), mouseArea(mouseArea) {}

        int id;

        UiComponent *component;

        /** Store a reference to the mouse area for delete purpose */
        Button *mouseArea;
    };

    // Todo objectiv with system implementation
    // struct SceneElementSystem : public System<Policy<ExecutionPolicy::Manual>, Own<SceneElement>, Need<MasterRenderer>, Talk<SceneSystem>>
    struct SceneElementSystem : public System<Own<SceneElement>>
    {
        SceneElementSystem(MasterRenderer* masterRenderer) : masterRenderer(masterRenderer)
        {
            setPolicy(ExecutionPolicy::Manual);
        }

        virtual void execute()
        {
            // parallelExecute(view<SceneElement>(), executeSceneElement, masterRenderer);

            for(const auto& element : view<SceneElement>())
            {
                // if(element->component != nullptr)
                //     element->component->render(masterRenderer);
            }
        }

        MasterRenderer *masterRenderer;
    };

    // Todo add all the logger thing to all those systems and doc too
    struct TickEvent
    {
        TickEvent(size_t duration) : tick(duration) {}

        size_t tick;
    };

    // Todo find and fix why this system doesn't work
    struct TickingSystem : public System<>
    {
        TickingSystem(size_t duration = 40) : tickDuration(duration)
        { 
            LOG_THIS_MEMBER("Ticking System");
            
            // Todo replace QDateTime with std::chrono
            // firstTickTime = std::chrono::high_resolution_clock::now();
            firstTickTime = QDateTime::currentMSecsSinceEpoch();
            secondTickTime = QDateTime::currentMSecsSinceEpoch();
        }

        ~TickingSystem() { LOG_THIS_MEMBER("Ticking System"); stop(); }

        inline void stop()
        {
            LOG_THIS_MEMBER("Ticking System");

            LOG_INFO("Ticking System", "Ticking system stopping ...");

            paused = false;
        }

        inline void pause()
        {
            LOG_THIS_MEMBER("Ticking System");

            paused = true;
        }

        inline void resume()
        {
            LOG_THIS_MEMBER("Ticking System");

            firstTickTime = QDateTime::currentMSecsSinceEpoch();
            secondTickTime = QDateTime::currentMSecsSinceEpoch();

            paused = false;
        }

        virtual void execute()
        {
            LOG_THIS_MEMBER("Ticking System");

            secondTickTime = QDateTime::currentMSecsSinceEpoch();
            
            while(not paused and ((secondTickTime - firstTickTime) >= static_cast<qint64>(tickDuration)))
            {
                firstTickTime += tickDuration;

                ecsRef->sendEvent(TickEvent{tickDuration});
            }
        }

        size_t tickDuration;

        // Todo change qint64 with std::chrono
        // std::chrono::high_resolution_clock::time_point firstTickTime, secondTickTime; 
        qint64 firstTickTime, secondTickTime; 
        bool paused = false;
    };

    // Todo make a FPS system that print the current FPS !

    struct FpsSystem : public System<Listener<TickEvent>, InitSys, StoragePolicy>
    {
        void init() override
        {
            auto sentence = makeSentence(ecsRef, 0, 0, {"0"});

            fpsTextId = sentence.entity.id;
        }

        void onEvent(const TickEvent& event) override
        {
            LOG_THIS_MEMBER("FactorySystem");

            accumulatedTick += event.tick;

            if (accumulatedTick >= 1000)
            {
                accumulatedTick %= 1000;

                auto rendererSys = ecsRef->getSystem<MasterRenderer>();

                if (not rendererSys)
                    return;

                auto currentNbOfFrames = rendererSys->nbRenderedFrames;

                auto res = currentNbOfFrames - lastNbOfFrames;

                auto fpsStr = Strfy() << res;

                lastNbOfFrames = currentNbOfFrames;

                // Print FPS
                ecsRef->sendEvent(OnTextChanged{fpsTextId, fpsStr.getData()});
            }
        }

        _unique_id fpsTextId;
        size_t accumulatedTick = 0;
        size_t lastNbOfFrames = 0;
    };

    struct OnClickGainGold { };

    struct OnGoldGain
    {
        OnGoldGain(int64_t gold) : gold(gold) {}

        int64_t gold;
    };

    struct GoldSystem : public System<Listener<OnClickGainGold>, Listener<OnGoldGain>, StoragePolicy, InitSys>
    {
        void init() override
        {
            auto sentence = makeSentence(ecsRef, 150, 20, {"0"});

            goldTextId = sentence.entity.id;
        }

        void onEvent(const OnClickGainGold&) override
        {
            this->gold += clickPower;

            auto goldStr = Strfy() << this->gold.load();

            ecsRef->sendEvent(OnTextChanged{goldTextId, goldStr.getData()});
        }

        void onEvent(const OnGoldGain& event) override
        {
            this->gold += event.gold;

            auto goldStr = Strfy() << this->gold.load();

            ecsRef->sendEvent(OnTextChanged{goldTextId, goldStr.getData()});
        }

        int64_t clickPower = 1;

        std::atomic<int64_t> gold {0};
        _unique_id goldTextId;
    };

    struct BuyFactory { };

    struct FactorySystem : public System<Listener<BuyFactory>, Listener<TickEvent>, StoragePolicy>
    {
        void onEvent(const BuyFactory&) override
        {
            LOG_THIS_MEMBER("FactorySystem");

            LOG_INFO("FactorySystem", "Bought a new factory");
            nbFactory += 1;
        }

        void onEvent(const TickEvent& event) override
        {
            LOG_THIS_MEMBER("FactorySystem");

            accumulatedTick += event.tick;

            while (accumulatedTick >= factoryProdDuration)
            {
                LOG_INFO("FactorySystem", "Factory produced gold");

                accumulatedTick -= factoryProdDuration;

                ecsRef->sendEvent(OnGoldGain{static_cast<int64_t>(nbFactory * factoryProdValue)});
            }
        }

        size_t accumulatedTick = 0;

        size_t nbFactory = 0;
        size_t factoryProdDuration = 1000;
        size_t factoryProdValue = 1;
    };

}

EditorWindow::EditorWindow(QWindow *parent) : QWindow(parent)
{
    setSurfaceType(QWindow::OpenGLSurface);

    mousePos = QPoint(0.0f, 0.0f);

    inputHandler = new Input();
}

EditorWindow::~EditorWindow()
{
    ticking = false;

    ecs.stop();

    if(inputHandler != nullptr)
        delete inputHandler;

    if(fontLoader != nullptr)
        delete fontLoader;

    if(contextMenu != nullptr)
        delete contextMenu;

    delete m_device;
}

void EditorWindow::render(QPainter *painter)
{
    Q_UNUSED(painter);
}

void EditorWindow::initialize()
{
	initializeOpenGLFunctions();

    // auto masterRenderer = ecs.getMasterRenderer();

    ecs.createSystem<TickingSystem>();

    ecs.createSystem<UiComponentSystem>();
    // ecs.createSystem<ButtonSystem>(masterRenderer);
    ecs.createSystem<TextureComponentSystem>();
    // sceneEcs.createSystem<SceneElementSystem>(masterRenderer); 

    ecs.createSystem<MouseClickSystem>(inputHandler);

    masterRenderer = ecs.createSystem<MasterRenderer>();

    ecs.createSystem<TerminalLogSystem>();

    masterRenderer->initialize(m_context);
    masterRenderer->setWindowSize(width(), height());

    //glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    masterRenderer->setWindowSize(640, 480);

    masterRenderer->registerShader("default", ":/shader/default.vs", ":/shader/default.fs");
    //masterRenderer.registerRederer<TextureRenderer>();

    masterRenderer->registerShader("gui", ":/shader/default.vs", ":/shader/default.fs");
    masterRenderer->registerShader("text", ":/shader/textrendering.vs", ":/shader/textrendering.fs");
    //masterRenderer.registerRederer<SentenceRenderer>();

    masterRenderer->registerShader("particle", ":/shader/particle.vs", ":/shader/particle.fs");
    //masterRenderer.registerRederer<ParticleRenderer>();

    masterRenderer->registerTexture("atlas", ":/res/tiles/TeclaEatsAtlas.png");
    masterRenderer->registerTexture("menu", ":/res/menu/Menu.png");
    masterRenderer->registerTexture("font", ":/res/font/font.png");
    masterRenderer->registerTexture("pigeon", ":/res/object/PigeonMockUp.png");

    //TODO register texture used in a Scene
    // and unload them when the scene is destroyed

    //TODO remove this
    masterRenderer->registerTexture("frame", ":/res/menu/frame.png");
    masterRenderer->registerTexture("menutest", ":/res/menu/menutest.png");
    masterRenderer->registerTexture("Menu2", ":/res/menu/Menu2.png");
    masterRenderer->registerTexture("slider", ":/res/object/slider.png");
    masterRenderer->registerTexture("cursor", ":/res/object/cursor.png");

    masterRenderer->registerTexture("TabTexture", ":/res/menu/NavyBlueTexture.png");
    masterRenderer->registerTexture("Light Blue", ":/res/menu/LightBlueTexture.png");

    fontLoader = new FontLoader("res/font/fontmap.ft");
    ecs.createSystem<SentenceSystem>(fontLoader);

    ecs.createSystem<FpsSystem>();

    auto goldSys = ecs.createSystem<GoldSystem>();

    ecs.createSystem<FactorySystem>();

    // Ecs task parenting

    // ecs.succeed<GoldSystem, FactorySystem>();
    // ecs.succeed<SentenceSystem, GoldSystem>();
    ecs.succeed<MouseClickSystem, TickingSystem>();

    ecs.succeed<MasterRenderer, MouseClickSystem>();

    ecs.dumbTaskflow();

    auto goldTextEntity = ecs.getEntity(goldSys->goldTextId);

    // ecs.createSystem<InputSystem>();

    screenEntity = ecs.createEntity();
    screenUi = ecs.attach<UiComponent>(screenEntity);
    screenUi->width = 400;
    screenUi->height = 400;
    screenUi->setZ(-1);

    auto goldTextUi = goldTextEntity->get<UiComponent>();

    goldTextUi->setTopAnchor(screenUi->top);
    goldTextUi->setLeftAnchor(screenUi->left);

    goldTextUi->setTopMargin(20);
    goldTextUi->setLeftMargin((screenUi->width / 2) - (goldTextUi->width / 2));

    // Todo
    // makeKeyInput(this, EditorWindow::quit);

    // optionTab = new editor::OptionTab(300, 1);

    // optionTab->setTopAnchor(screenUi->top);
    // optionTab->setRightAnchor(screenUi->right);
    // optionTab->setBottomAnchor(screenUi->bottom);

    auto sceneEntity = ecs.createEntity();
    sceneEntityC = ecs.attach<UiComponent>(sceneEntity);

    sceneEntityC->setWidth(200);
    sceneEntityC->setHeight(40);

    sceneEntityC->setX(20);
    sceneEntityC->setY(20);

    // sceneEntityC->setLeftAnchor(screenUi->left);
    // sceneEntityC->setRightAnchor(optionTab->left);
    // sceneEntityC->setTopAnchor(screenUi->top);
    // sceneEntityC->setBottomAnchor(screenUi->bottom);

    // makeMouseArea(&ecs, sceneEntityC, this, EditorWindow::openContextMenu, EditorWindow::closeContextMenu);

    // [Start] Context menu UI

    // contextMenu = new editor::ContextMenu(ecs, fontLoader, "TabTexture", [=](const UiComponentType& type) {this->addElement(type);});

    //auto contextMenuEntity = ecs.createEntity();
    //contextMenu = ecs.attach<TextureComponent>(contextMenuEntity, 250, 100, "TabTexture");

    // contextMenu->hide();
    
    // [End] Context menu UI

    // std::cout << b1->width << std::endl;
    // std::cout << b1->pos.x << std::endl;

    ecs.attach<TextureComponent>(sceneEntity, "frame");

    std::cout << sceneEntityC->frame.w << std::endl;

    makeUiTexture(&ecs, 80, 20, "pigeon");

    auto e = makeUiTexture(&ecs, 160, 90, "menu");
    auto c = e.get<UiComponent>();

    c->setBottomAnchor(screenUi->bottom);
    c->setRightAnchor(screenUi->right);

    // auto testingString = "Testing";

    // ecs.attach<MouseClickComponent>(sceneEntity, makeCallable<LogInfoEvent>(testingString, "Clicked on component"));
    ecs.attach<MouseClickComponent>(sceneEntity, makeCallable<OnClickGainGold>());

    auto factoryCreationList = makeUiTexture(&ecs, 64, 32, "frame");
    auto factoryCreationListEntity = factoryCreationList.entity;
    auto factoryCreationListPos = factoryCreationList.get<UiComponent>();

    factoryCreationListPos->setBottomAnchor(screenUi->bottom);
    factoryCreationListPos->setLeftAnchor(screenUi->left);

    ecs.attach<MouseClickComponent>(factoryCreationListEntity, makeCallable<BuyFactory>());

    // ecs.attach<SentenceText>(sceneEntity, "Hello there !");

    makeSentence(&ecs, 20, 250, {"\"Hello_World\": Test?!"});
    
    // ticking = true;
    // std::thread t (&EditorWindow::tick, this);

    // makeSentence(&ecs, 20, 150, {"\"Hello_World\": Test?!"});

    // t.detach();

    ecs.start();
}

void EditorWindow::render()
{
    currentTime = QDateTime::currentMSecsSinceEpoch();
    static auto lastTime = QDateTime::currentMSecsSinceEpoch();

    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    glClearColor(0.1f, 0.3f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    currentTime = QDateTime::currentMSecsSinceEpoch();

    if(screenUi->width != width())
    {
        screenUi->setWidth(width());
        masterRenderer->setWindowSize(width(), height());
    }
        
    if(screenUi->height != height())
    {
        screenUi->setHeight(height());
        masterRenderer->setWindowSize(width(), height());
    }

    masterRenderer->setCurrentTime(currentTime);

    // InputSystem::system()->updateState(inputHandler, float(currentTime - lastTime) / 1000);

    // renderUi();

    masterRenderer->renderAll();
    // sceneEcs.executeAll();
    
    // masterRenderer->execute();

    inputHandler->updateInput(float(currentTime - lastTime) / 1000);

    lastTime = currentTime;

    nbFrame++;
}

void EditorWindow::setAnimating(bool animating)
{
    m_animating = animating;

    if (animating)
        renderLater();
}

void EditorWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
        event->ignore();
    else
    {
        inputHandler->registerKeyInput((Qt::Key)event->key(), Input::InputState::KEYPRESSED);
    }
}

void EditorWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
        event->ignore();
    else
        inputHandler->registerKeyInput((Qt::Key)event->key(), Input::InputState::KEYRELEASED);
}

void EditorWindow::mouseMoveEvent(QMouseEvent *event)
{
    QPoint mouseDelta;

    mouseDelta.setX((mousePos.x() - event->pos().x()) * xSensitivity);
    mouseDelta.setY((event->pos().y() - mousePos.y()) * ySensitivity); // reversed since y-coordinates go from bottom to top

    inputHandler->registerMouseMove(event->pos(), mouseDelta);

    mousePos = event->pos();
}

void EditorWindow::mousePressEvent(QMouseEvent *event)
{
    mousePos = event->pos();

    if(event->button() != Qt::NoButton) //TODO: check why i can t grab a button even
    {
        inputHandler->registerMouseInput((Qt::MouseButton)event->button(), Input::InputState::MOUSEPRESS);
    }
}

void EditorWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() != Qt::NoButton)
        inputHandler->registerMouseInput((Qt::MouseButton)event->button(), Input::InputState::MOUSERELEASE);
}

void EditorWindow::wheelEvent(QWheelEvent*)
{
}

void EditorWindow::renderLater()
{
    requestUpdate();
}

void EditorWindow::renderNow()
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
}

bool EditorWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::UpdateRequest:
        renderNow();
        return true;
    default:
        return QWindow::event(event);
    }
}

void EditorWindow::exposeEvent(QExposeEvent *event)
{
    Q_UNUSED(event);

    if (isExposed())
        renderNow();
}

/**
 * @brief Ui callback function that open the content window to add elements to the scene
 * 
 * @param inputHandler A pointer to the input handler object
 * 
 * Open a context menu to add element to the scene when the user right click in the scene space in the editor
 * 
 * @see closeContextMenu
 */
void EditorWindow::openContextMenu(Input* inputHandler, double...)
{
    static bool pressed = false;

    if(inputHandler->isButtonPressed(Qt::LeftButton))
    {
        const auto& mousePos = inputHandler->getMousePos();

        if(not contextMenu->inBound(mousePos.x(), mousePos.y()))
            contextMenu->hide();
    }

    if(inputHandler->isButtonPressed(Qt::RightButton) && !pressed)
    {
        pressed = true;
    }

    if(not inputHandler->isButtonPressed(Qt::RightButton) && pressed)
    {
        const auto& mousePos = inputHandler->getMousePos();

        auto xPos = mousePos.x();
        auto yPos = mousePos.y();

        if(contextMenu->width + xPos > sceneEntityC->width)
            xPos -= contextMenu->width;

        if(contextMenu->height + yPos > sceneEntityC->height)
            yPos -= contextMenu->height;

        contextMenu->setX(xPos);
        contextMenu->setY(yPos);

        contextMenu->show();

        pressed = false;
    }

}

void EditorWindow::closeContextMenu(Input* inputHandler, double)
{
    if(inputHandler->isButtonPressed(Qt::LeftButton) or inputHandler->isButtonPressed(Qt::RightButton))
    {
        contextMenu->hide();
    }
}

void EditorWindow::addElement(const UiComponentType& type)
{
    static int index = 0;
    auto ent = sceneEcs.createEntity();
    auto ecsRef = &sceneEcs;
    UiComponent *component = nullptr;
    int componentX = 0, componentY = 0;
    Button *mouseArea = nullptr;

    // TODO: take the correct coord of the context menu (context menu can show up from the top of the cursor if their is not enough space in the bottom of the screen)
    if(contextMenu != nullptr)
    {
    	componentX = static_cast<UiSize>(contextMenu->pos.x);
        componentY = static_cast<UiSize>(contextMenu->pos.y);
    }

    switch(type)
    {
    case UiComponentType::BUTTON:
        // sceneEcs.attach<SceneElement>(index, new Button());
        break;

    case UiComponentType::TEXTURE:
        // component = new TextureComponent(50, 50, "TabTexture");
        component->setZ(index);

        component->setTopAnchor(sceneEntityC->top);
        component->setLeftAnchor(sceneEntityC->left);

        component->setTopMargin(componentY - 25);
        component->setLeftMargin(componentX - 25);

        // Todo
        // mouseArea = new Button([=](Input*, double){ delete component; }, component->frame);
        mouseArea = new Button(ecsRef, [component, mouseArea, ecsRef, ent](Input*, double){ ecsRef->dettach<SceneElement>(ent); delete component; delete mouseArea; }, component->frame);
        // mouseArea = new Button([=](Input*, double){ this->openInOption<TextureComponent>(component); }, component->frame);
        mouseArea->setZ(component->pos.z + 1);
        break;

    case UiComponentType::TEXT:
        // component = new Sentence({"Text"}, 2.0f, fontLoader);

        // component->setTopAnchor(sceneEntityC->top);
        // component->setLeftAnchor(sceneEntityC->left);

        // component->setTopMargin(componentY - component->height / 2.0f);
        // component->setLeftMargin(componentX - component->width / 2.0f);

        // mouseArea = new Button(ecsRef, [=](Input*, double){ this->openInOption<Sentence>(component); }, component->frame);
        // mouseArea->setZ(component->pos.z + 1);
        break;

    case UiComponentType::TEXTINPUT:
        component = new TextInput(UiFrame{0.0f, 0.0f, 10.0f, 100.0f, 50.0f}, "TabTexture", fontLoader, [](const std::string& text){std::cout << text << std::endl;});

        component->setTopAnchor(sceneEntityC->top);
        component->setLeftAnchor(sceneEntityC->left);

        component->setTopMargin(componentY - component->height / 2.0f);
        component->setLeftMargin(componentX - component->width / 2.0f);

        mouseArea = new Button(ecsRef, [=](Input*, double){ this->openInOption<Sentence>(component); }, component->frame);
        mouseArea->setZ(component->pos.z - 1);
        break;

    case UiComponentType::LIST:

        break;

    case UiComponentType::PREFAB:

        break;

    default:

        break;
    }

    sceneEcs.attach<SceneElement>(ent, index, component, mouseArea);
    index++;

    contextMenu->hide();
}

template <typename SceneElementType>
void EditorWindow::openInOption(UiComponent* component)
{
    // optionTab->clear();
    // printOption(optionTab, static_cast<SceneElementType>(component));
}

void EditorWindow::renderUi()
{
    // ecs.getMasterRenderer()->render(optionTab);

/*
    for(auto& child : sceneEcs.view<SceneElement>())
    {
        if(child.component != nullptr)
            child.component->render(&masterRenderer);
    }
*/

/*
    // Todo use the master renderer to render all the texture component but using only one shader binding 
    for(auto& texture : ecs.view<TextureComponent>())
    {
        masterRenderer.render(&texture);
    }

    for(auto& button : ecs.view<Button>())
    {
       masterRenderer.render(&button);
    }

    for(auto& sentence : ecs.view<Sentence>()) //TODO set a note about how auto& is important to pass by ref and not create a copy which is costy 
    {
        masterRenderer.render(&sentence);
    }
*/
}

//TODO make a tick object that take tick function and run in background when you start up the engine
void EditorWindow::tick()
{
    LOG_THIS_MEMBER(DOM);

    // auto currentTickTime = QDateTime::currentMSecsSinceEpoch();
    // auto lastTickTime = QDateTime::currentMSecsSinceEpoch();

    // size_t accumulatedTickCount = 0;


    // while(ticking)
    // {
    //     // LOG_INFO(DOM, nbFrame);

    //     lastTickTime = QDateTime::currentMSecsSinceEpoch();

    //     accumulatedTickCount += 40;

    //     while (accumulatedTickCount >= 1000)
    //     {
    //         accumulatedTickCount -= 1000;

    //         std::cout << nbFrame << std::endl;  
    //         nbFrame = 0;
    //     }
        
    //     // Todo remove this when the Ticking system works
    //     ecs.sendEvent(TickEvent{900});

    //     // //Animation tick loop
    //     // for(int i = AnimationComponent::runningQueue.size() - 1; i >= 0; i--) 
    //     // {
    //     //     AnimationComponent::runningQueue[i]->tick(40); // tickRate
            
    //     //     if(!AnimationComponent::runningQueue[i]->isRunning())
    //     //     {
    //     //         //delete AnimationComponent::runningQueue[i]; // TODO check where to put this
    //     //         AnimationComponent::runningQueue.erase(AnimationComponent::runningQueue.begin() + i);
    //     //     }
    //     // }

    //     currentTickTime = QDateTime::currentMSecsSinceEpoch();
    //     std::this_thread::sleep_for(std::chrono::milliseconds(20 - (currentTickTime - lastTickTime)));
    // }
}

void EditorWindow::quit(Input* inputHandler, double...)
{
    if(inputHandler->isKeyPressed(Qt::Key_Escape))
        emit quitApp();
}