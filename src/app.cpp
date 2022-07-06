#include "app.h"

#include <QDateTime>

#include "Engine/logger.h"
#include "Engine/serialization.h" 
#include "Engine/configuration.h"
#include "Engine/Scene/sceneloader.h"

#include "UI/button.h"
#include "UI/texture.h"
#include "UI/textinput.h"

#include "Editor/Gui/contextmenu.h"
#include "Editor/Gui/optiontab.h"

// TODO create a find function in ECS

namespace
{
    const char * DOM = "Editor window";

    struct SceneElement
    {
        SceneElement(int id, UiComponent *component, Button *mouseArea) : id(id), component(component), mouseArea(mouseArea) {}

        int id;

        UiComponent *component;

        /** Store a reference to the mouse area for delete purpose */
        Button *mouseArea;
    };
}

EditorWindow::EditorWindow(QWindow *parent) : QWindow(parent)
{
    // Enable log in console
    auto terminalSink = pg::Logger::registerSink<pg::TerminalSink>(true);
    //TODO fix FilterFile
    //terminalSink->addFilter("Input Filter", new Logger::LogSink::FilterScope("Input"));
    terminalSink->addFilter("Serializer Filter", new Logger::LogSink::FilterFile("src/Engine/serialization.cpp"));
    terminalSink->addFilter("Configuration Filter", new Logger::LogSink::FilterFile("src/Engine/configuration.cpp"));
    terminalSink->addFilter("Log Level Filter", new Logger::LogSink::FilterLogLevel(Logger::InfoLevel::log));

    setSurfaceType(QWindow::OpenGLSurface);

    mousePos = QPoint(0.0f, 0.0f);

    inputHandler = new Input();
}

EditorWindow::~EditorWindow()
{
    ticking = false;

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

    masterRenderer.initialize(m_context);
    masterRenderer.setWindowSize(width(), height());

    //glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    masterRenderer.setWindowSize(640, 480);

    masterRenderer.registerShader("default", ":/shader/default.vs", ":/shader/default.fs");
    //masterRenderer.registerRederer<TextureRenderer>();

    masterRenderer.registerShader("gui", ":/shader/default.vs", ":/shader/default.fs");
    masterRenderer.registerShader("text", ":/shader/textrendering.vs", ":/shader/textrendering.fs");
    //masterRenderer.registerRederer<SentenceRenderer>();

    masterRenderer.registerShader("particle", ":/shader/particle.vs", ":/shader/particle.fs");
    //masterRenderer.registerRederer<ParticleRenderer>();

    masterRenderer.registerTexture("atlas", ":/res/tiles/TeclaEatsAtlas.png");
    masterRenderer.registerTexture("menu", ":/res/menu/Menu.png");
    masterRenderer.registerTexture("font", ":/res/font/font.png");
    masterRenderer.registerTexture("pigeon", ":/res/object/PigeonMockUp.png");

    //TODO register texture used in a Scene
    // and unload them when the scene is destroyed

    //TODO remove this
    masterRenderer.registerTexture("frame", ":/res/menu/frame.png");
    masterRenderer.registerTexture("menutest", ":/res/menu/menutest.png");
    masterRenderer.registerTexture("Menu2", ":/res/menu/Menu2.png");
    masterRenderer.registerTexture("slider", ":/res/object/slider.png");
    masterRenderer.registerTexture("cursor", ":/res/object/cursor.png");

    masterRenderer.registerTexture("TabTexture", ":/res/menu/NavyBlueTexture.png");

    fontLoader = new FontLoader("res/font/fontmap.ft");

    screenEntity = ecs.createEntity();
    screenUi = ecs.attach<UiComponent>(screenEntity);
    screenUi->width = 400;
    screenUi->height = 400;
    screenUi->setZ(-1);

    makeKeyInput(this, EditorWindow::quit);

    optionTab = new editor::OptionTab(300, 1);

    optionTab->setTopAnchor(screenUi->top);
    optionTab->setRightAnchor(screenUi->right);
    optionTab->setBottomAnchor(screenUi->bottom);

    auto sceneEntity = ecs.createEntity();
    sceneEntityC = ecs.attach<UiComponent>(sceneEntity);

    sceneEntityC->setLeftAnchor(screenUi->left);
    sceneEntityC->setRightAnchor(optionTab->left);
    sceneEntityC->setTopAnchor(screenUi->top);
    sceneEntityC->setBottomAnchor(screenUi->bottom);

    makeMouseArea(sceneEntityC, this, EditorWindow::openContextMenu, EditorWindow::closeContextMenu);

    // [Start] Context menu UI

    contextMenu = new editor::ContextMenu(ecs, fontLoader, "TabTexture", [=](const UiComponentType& type) {this->addElement(type);});

    //auto contextMenuEntity = ecs.createEntity();
    //contextMenu = ecs.attach<TextureComponent>(contextMenuEntity, 250, 100, "TabTexture");

    contextMenu->hide();
    
    // [End] Context menu UI

    // std::cout << b1->width << std::endl;
    // std::cout << b1->pos.x << std::endl;

    std::cout << sceneEntityC->frame.w << std::endl;
    
    ticking = true;
    std::thread t (&EditorWindow::tick, this);

    t.detach();
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
        masterRenderer.setWindowSize(width(), height());
    }
        
    if(screenUi->height != height())
    {
        screenUi->setHeight(height());
        masterRenderer.setWindowSize(width(), height());
    }

    masterRenderer.setCurrentTime(currentTime);

    InputSystem::system()->updateState(inputHandler, float(currentTime - lastTime) / 1000);

    renderUi();

    inputHandler->updateInput(float(currentTime - lastTime) / 1000);

    lastTime = currentTime;
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
    auto entRef = &ent;
    auto ecsRef = &sceneEcs;
    UiComponent *component = nullptr;
    int componentX = 0, componentY = 0;
    Button *mouseArea = nullptr;

    // TODO: take the correct coord of the context menu (context menu can show up from the top of the cursor if their is not enough space in the bottom of the screen)
    if(contextMenu != nullptr)
    {
    	componentX = contextMenu->pos.x;
        componentY = contextMenu->pos.y;
    }

    switch(type)
    {
    case UiComponentType::BUTTON:
        // sceneEcs.attach<SceneElement>(index, new Button());
        break;

    case UiComponentType::TEXTURE:
        component = new TextureComponent(50, 50, "TabTexture");

        component->setTopAnchor(sceneEntityC->top);
        component->setLeftAnchor(sceneEntityC->left);

        component->setTopMargin(componentY - 25);
        component->setLeftMargin(componentX - 25);

        // Todo
        // mouseArea = new Button([=](Input*, double){ delete component; }, component->frame);
        mouseArea = new Button([component, mouseArea, ecsRef, entRef](Input*, double){ delete component; delete mouseArea; ecsRef->dettach<SceneElement>(*entRef); }, component->frame);
        // mouseArea = new Button([=](Input*, double){ this->openInOption<TextureComponent>(component); }, component->frame);
        mouseArea->setZ(component->pos.z + 1);
        break;

    case UiComponentType::TEXT:
        component = new Sentence({"Text"}, 2.0f, fontLoader);

        component->setTopAnchor(sceneEntityC->top);
        component->setLeftAnchor(sceneEntityC->left);

        component->setTopMargin(componentY - component->height / 2.0f);
        component->setLeftMargin(componentX - component->width / 2.0f);

        mouseArea = new Button([=](Input*, double){ this->openInOption<Sentence>(component); }, component->frame);
        mouseArea->setZ(component->pos.z + 1);
        break;

    case UiComponentType::TEXTINPUT:
        component = new TextInput(UiFrame{0.0f, 0.0f, 10.0f, 100.0f, 50.0f}, "TabTexture", fontLoader, [](const std::string& text){std::cout << text << std::endl;});

        component->setTopAnchor(sceneEntityC->top);
        component->setLeftAnchor(sceneEntityC->left);

        component->setTopMargin(componentY - component->height / 2.0f);
        component->setLeftMargin(componentX - component->width / 2.0f);

        mouseArea = new Button([=](Input*, double){ this->openInOption<Sentence>(component); }, component->frame);
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

template<typename SceneElementType>
void EditorWindow::openInOption(UiComponent* component)
{
    std::cout << "Clicked on component " << std::endl;
}

void EditorWindow::renderUi()
{
    masterRenderer.render(optionTab);

    for(auto& child : sceneEcs.view<SceneElement>())
    {
        if(child.component != nullptr)
            child.component->render(&masterRenderer);
    }

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
}

//TODO make a tick object that take tick function and run in background when you start up the engine
void EditorWindow::tick()
{
    LOG_THIS_MEMBER(DOM);

    auto currentTickTime = QDateTime::currentMSecsSinceEpoch();
    auto lastTickTime = QDateTime::currentMSecsSinceEpoch();

    while(ticking)
    {
        lastTickTime = QDateTime::currentMSecsSinceEpoch();

        //Animation tick loop
        for(int i = AnimationComponent::runningQueue.size() - 1; i >= 0; i--) 
        {
            AnimationComponent::runningQueue[i]->tick(40); // tickRate
            
            if(!AnimationComponent::runningQueue[i]->isRunning())
            {
                //delete AnimationComponent::runningQueue[i]; // TODO check where to put this
                AnimationComponent::runningQueue.erase(AnimationComponent::runningQueue.begin() + i);
            }
        }

        currentTickTime = QDateTime::currentMSecsSinceEpoch();
        std::this_thread::sleep_for(std::chrono::milliseconds(40 - (currentTickTime - lastTickTime)));
    }
}

void EditorWindow::quit(Input* inputHandler, double...)
{
    if(inputHandler->isKeyPressed(Qt::Key_Escape))
        emit quitApp();
}