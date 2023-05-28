// #include "app.h"

// // Todo remove
// #include <QDateTime>

// #include <chrono>

// #include "Engine/logger.h"
// #include "Engine/serialization.h" 
// #include "Engine/configuration.h"

// #include "Engine/Scene/sceneloader.h"

// #include "Engine/ECS/loggersystem.h"

// #include "UI/button.h"
// #include "UI/texture.h"
// #include "UI/textinput.h"

// #include "Editor/Gui/contextmenu.h"
// #include "Editor/Gui/optiontab.h"

// #include "Scene/scenemanager.h"

// #include "GameElements/Systems/basicsystems.h"

// // TODO create a find function in ECS

// namespace
// {
//     static constexpr char const * DOM = "Editor window";
// }

// EditorWindow::EditorWindow(QWindow *parent) : QWindow(parent)
// {
//     setSurfaceType(QWindow::OpenGLSurface);

//     mousePos = QPoint(0.0f, 0.0f);

//     inputHandler = new Input();
// }

// EditorWindow::~EditorWindow()
// {
//     ticking = false;

//     ecs.stop();

//     if(inputHandler != nullptr)
//         delete inputHandler;

//     if(fontLoader != nullptr)
//         delete fontLoader;

//     if(contextMenu != nullptr)
//         delete contextMenu;

//     delete m_device;
// }

// void EditorWindow::render(QPainter *painter)
// {
//     Q_UNUSED(painter);
// }

// void EditorWindow::initialize()
// {
// 	initializeOpenGLFunctions();

//     // auto masterRenderer = ecs.getMasterRenderer();

//     ecs.createSystem<TickingSystem>();

//     ecs.createSystem<UiComponentSystem>();
//     // ecs.createSystem<ButtonSystem>(masterRenderer);
//     ecs.createSystem<TextureComponentSystem>();
//     // sceneEcs.createSystem<SceneElementSystem>(masterRenderer); 

//     ecs.createSystem<MouseLeftClickSystem>(inputHandler);
//     ecs.createSystem<MouseRightClickSystem>(inputHandler);

//     ecs.createSystem<MouseLeaveClickSystem>(inputHandler);

//     masterRenderer = ecs.createSystem<MasterRenderer>();

//     ecs.createSystem<TerminalLogSystem>();

//     masterRenderer->initialize(m_context);
//     masterRenderer->setWindowSize(width(), height());

//     //glEnable(GL_DEPTH_TEST);
//     glEnable(GL_BLEND);
//     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//     masterRenderer->setWindowSize(640, 480);

//     masterRenderer->registerShader("default", ":/shader/default.vs", ":/shader/default.fs");
//     //masterRenderer.registerRederer<TextureRenderer>();

//     masterRenderer->registerShader("gui", ":/shader/default.vs", ":/shader/default.fs");
//     masterRenderer->registerShader("text", ":/shader/textrendering.vs", ":/shader/textrendering.fs");
//     //masterRenderer.registerRederer<SentenceRenderer>();

//     masterRenderer->registerShader("particle", ":/shader/particle.vs", ":/shader/particle.fs");
//     //masterRenderer.registerRederer<ParticleRenderer>();

//     masterRenderer->registerTexture("atlas", ":/res/tiles/TeclaEatsAtlas.png");
//     masterRenderer->registerTexture("menu", ":/res/menu/Menu.png");
//     masterRenderer->registerTexture("font", ":/res/font/font.png");
//     masterRenderer->registerTexture("pigeon", ":/res/object/PigeonMockUp.png");

//     //TODO register texture used in a Scene
//     // and unload them when the scene is destroyed

//     //TODO remove this
//     masterRenderer->registerTexture("frame", ":/res/menu/frame.png");
//     masterRenderer->registerTexture("menutest", ":/res/menu/menutest.png");
//     masterRenderer->registerTexture("Menu2", ":/res/menu/Menu2.png");
//     masterRenderer->registerTexture("slider", ":/res/object/slider.png");
//     masterRenderer->registerTexture("cursor", ":/res/object/cursor.png");

//     masterRenderer->registerTexture("TabTexture", ":/res/menu/NavyBlueTexture.png");
//     masterRenderer->registerTexture("Light Blue", ":/res/menu/LightBlueTexture.png");

//     fontLoader = new FontLoader("res/font/fontmap.ft");
//     ecs.createSystem<SentenceSystem>(fontLoader);

//     ecs.createSystem<FpsSystem>();

//     auto goldSys = ecs.createSystem<GoldSystem>();

//     ecs.createSystem<FactorySystem>();

//     ecs.createSystem<SceneElementSystem>();

//     // Ecs task parenting

//     // ecs.succeed<GoldSystem, FactorySystem>();
//     // ecs.succeed<SentenceSystem, GoldSystem>();
//     ecs.succeed<MouseRightClickSystem, TickingSystem>();
//     ecs.succeed<MouseLeftClickSystem, TickingSystem>();

//     ecs.succeed<UiComponentSystem, MouseRightClickSystem>();
//     ecs.succeed<UiComponentSystem, MouseLeftClickSystem>();

//     ecs.succeed<MasterRenderer, UiComponentSystem>();

//     ecs.createSystem<editor::ContextMenu>();

//     // contextMenu = new editor::ContextMenu(ecs);

//     ecs.dumbTaskflow();

//     auto goldTextEntity = ecs.getEntity(goldSys->goldTextId);

//     // ecs.createSystem<InputSystem>();

//     screenEntity = ecs.createEntity();
//     screenUi = ecs.attach<UiComponent>(screenEntity);
//     screenUi->width = 400;
//     screenUi->height = 400;
//     screenUi->setZ(-1);

//     auto goldTextUi = goldTextEntity->get<UiComponent>();

//     goldTextUi->setTopAnchor(screenUi->top);
//     goldTextUi->setLeftAnchor(screenUi->left);

//     goldTextUi->setTopMargin(20);
//     goldTextUi->setLeftMargin((screenUi->width / 2) - (goldTextUi->width / 2));

//     // Todo
//     // makeKeyInput(this, EditorWindow::quit);

//     // optionTab = new editor::OptionTab(300, 1);

//     // optionTab->setTopAnchor(screenUi->top);
//     // optionTab->setRightAnchor(screenUi->right);
//     // optionTab->setBottomAnchor(screenUi->bottom);

//     auto entityTabEntity = makeUiTexture(&ecs, 300, 200, "TabTexture");
//     auto entityTabEntityUiC = entityTabEntity.get<UiComponent>();

//     entityTabEntityUiC->setTopAnchor(screenUi->top);
//     entityTabEntityUiC->setLeftAnchor(screenUi->left);
//     // entityTabEntityUiC->setRightAnchor(screenUi->right);
//     entityTabEntityUiC->setBottomAnchor(screenUi->bottom);

//     auto sceneEntity = ecs.createEntity();
//     sceneEntityC = ecs.attach<UiComponent>(sceneEntity);

//     sceneEntityC->setLeftAnchor(entityTabEntityUiC->right);
//     sceneEntityC->setRightAnchor(screenUi->right);
//     sceneEntityC->setTopAnchor(screenUi->top);
//     sceneEntityC->setBottomAnchor(screenUi->bottom);

//     ecs.attach<MouseRightClickComponent>(sceneEntity, makeCallable<editor::ShowContextMenu>(inputHandler, sceneEntityC));

//     // ;

//     // makeMouseArea(&ecs, sceneEntityC, this, EditorWindow::openContextMenu, EditorWindow::closeContextMenu);

//     // [Start] Context menu UI

//     // contextMenu = new editor::ContextMenu(ecs, fontLoader, "TabTexture", [=](const UiComponentType& type) {this->addElement(type);});

//     //auto contextMenuEntity = ecs.createEntity();
//     //contextMenu = ecs.attach<TextureComponent>(contextMenuEntity, 250, 100, "TabTexture");

//     // contextMenu->hide();
    
//     // [End] Context menu UI

//     // std::cout << b1->width << std::endl;
//     // std::cout << b1->pos.x << std::endl;

//     // ecs.attach<TextureComponent>(sceneEntity, "frame");
//     // ecs.attach<MouseLeftClickComponent>(sceneEntity, makeCallable<OnClickGainGold>());

//     std::cout << sceneEntityC->frame.w << std::endl;

//     makeUiTexture(&ecs, 80, 20, "pigeon");

//     auto e = makeUiTexture(&ecs, 160, 90, "menu");
//     auto c = e.get<UiComponent>();

//     c->setBottomAnchor(screenUi->bottom);
//     c->setRightAnchor(screenUi->right);

//     // auto testingString = "Testing";

//     // ecs.attach<MouseLeftClickComponent>(sceneEntity, makeCallable<LogInfoEvent>(testingString, "Clicked on component"));

//     auto factoryCreationList = makeUiTexture(&ecs, 64, 32, "frame");
//     auto factoryCreationListEntity = factoryCreationList.entity;
//     auto factoryCreationListPos = factoryCreationList.get<UiComponent>();

//     factoryCreationListPos->setBottomAnchor(screenUi->bottom);
//     factoryCreationListPos->setLeftAnchor(screenUi->left);

//     ecs.attach<MouseLeftClickComponent>(factoryCreationListEntity, makeCallable<BuyFactory>());

//     // ecs.attach<SentenceText>(sceneEntity, "Hello there !");

//     makeSentence(&ecs, 20, 250, {"\"Hello_World\": Test?!"});
    
//     // ticking = true;
//     // std::thread t (&EditorWindow::tick, this);

//     // makeSentence(&ecs, 20, 150, {"\"Hello_World\": Test?!"});

//     // t.detach();

//     ecs.start();

//     // Todo
//     // glEnable(GL_DEPTH_TEST);
// }

// void EditorWindow::render()
// {
//     currentTime = QDateTime::currentMSecsSinceEpoch();
//     static auto lastTime = QDateTime::currentMSecsSinceEpoch();

//     const qreal retinaScale = devicePixelRatio();
//     glViewport(0, 0, width() * retinaScale, height() * retinaScale);

//     glClearColor(0.1f, 0.3f, 0.7f, 1.0f);
//     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//     currentTime = QDateTime::currentMSecsSinceEpoch();

//     if(screenUi->width != width())
//     {
//         screenUi->setWidth(width());
//         masterRenderer->setWindowSize(width(), height());
//     }
        
//     if(screenUi->height != height())
//     {
//         screenUi->setHeight(height());
//         masterRenderer->setWindowSize(width(), height());
//     }

//     masterRenderer->setCurrentTime(currentTime);

//     // InputSystem::system()->updateState(inputHandler, float(currentTime - lastTime) / 1000);

//     // renderUi();

//     masterRenderer->renderAll();
//     // sceneEcs.executeAll();
    
//     // masterRenderer->execute();

//     inputHandler->updateInput(float(currentTime - lastTime) / 1000);

//     lastTime = currentTime;

//     nbFrame++;
// }

// void EditorWindow::setAnimating(bool animating)
// {
//     m_animating = animating;

//     if (animating)
//         renderLater();
// }

// void EditorWindow::keyPressEvent(QKeyEvent *event)
// {
//     if (event->isAutoRepeat())
//         event->ignore();
//     else
//     {
//         inputHandler->registerKeyInput((Qt::Key)event->key(), Input::InputState::KEYPRESSED);
//     }
// }

// void EditorWindow::keyReleaseEvent(QKeyEvent *event)
// {
//     if (event->isAutoRepeat())
//         event->ignore();
//     else
//         inputHandler->registerKeyInput((Qt::Key)event->key(), Input::InputState::KEYRELEASED);
// }

// void EditorWindow::mouseMoveEvent(QMouseEvent *event)
// {
//     QPoint mouseDelta;

//     mouseDelta.setX((mousePos.x() - event->pos().x()) * xSensitivity);
//     mouseDelta.setY((event->pos().y() - mousePos.y()) * ySensitivity); // reversed since y-coordinates go from bottom to top

//     inputHandler->registerMouseMove(event->pos(), mouseDelta);

//     mousePos = event->pos();
// }

// void EditorWindow::mousePressEvent(QMouseEvent *event)
// {
//     mousePos = event->pos();

//     if(event->button() != Qt::NoButton) //TODO: check why i can t grab a button even
//     {
//         inputHandler->registerMouseInput((Qt::MouseButton)event->button(), Input::InputState::MOUSEPRESS);
//     }
// }

// void EditorWindow::mouseReleaseEvent(QMouseEvent *event)
// {
//     if(event->button() != Qt::NoButton)
//         inputHandler->registerMouseInput((Qt::MouseButton)event->button(), Input::InputState::MOUSERELEASE);
// }

// void EditorWindow::wheelEvent(QWheelEvent*)
// {
// }

// void EditorWindow::renderLater()
// {
//     requestUpdate();
// }

// void EditorWindow::renderNow()
// {
//     if (!isExposed())
//         return;

//     bool needsInitialize = false;

//     if (!m_context) {
//         m_context = new QOpenGLContext(this);
//         m_context->setFormat(requestedFormat());
//         m_context->create();

//         needsInitialize = true;
//     }

//     m_context->makeCurrent(this);

//     if (needsInitialize) {
//         initializeOpenGLFunctions();
//         initialize();
//     }

//     render();

//     m_context->swapBuffers(this);
// }

// bool EditorWindow::event(QEvent *event)
// {
//     switch (event->type()) {
//     case QEvent::UpdateRequest:
//         renderNow();
//         return true;
//     default:
//         return QWindow::event(event);
//     }
// }

// void EditorWindow::exposeEvent(QExposeEvent *event)
// {
//     Q_UNUSED(event);

//     if (isExposed())
//         renderNow();
// }


// void EditorWindow::addElement(const UiComponentType& type)
// {
//     static int index = 0;
//     auto ent = sceneEcs.createEntity();
//     auto ecsRef = &sceneEcs;
//     UiComponent *component = nullptr;
//     int componentX = 0, componentY = 0;
//     Button *mouseArea = nullptr;

//     // TODO: take the correct coord of the context menu (context menu can show up from the top of the cursor if their is not enough space in the bottom of the screen)
//     if(contextMenu != nullptr)
//     {
//     	// componentX = static_cast<UiSize>(contextMenu->pos.x);
//         // componentY = static_cast<UiSize>(contextMenu->pos.y);
//     }

//     switch(type)
//     {
//     case UiComponentType::BUTTON:
//         // sceneEcs.attach<SceneElement>(index, new Button());
//         break;

//     case UiComponentType::TEXTURE:
//         // component = new TextureComponent(50, 50, "TabTexture");
//         component->setZ(index);

//         component->setTopAnchor(sceneEntityC->top);
//         component->setLeftAnchor(sceneEntityC->left);

//         component->setTopMargin(componentY - 25);
//         component->setLeftMargin(componentX - 25);

//         // Todo
//         // mouseArea = new Button([=](Input*, double){ delete component; }, component->frame);
//         mouseArea = new Button(ecsRef, [component, mouseArea, ecsRef, ent](Input*, double){ ecsRef->dettach<SceneElement>(ent); delete component; delete mouseArea; }, component->frame);
//         // mouseArea = new Button([=](Input*, double){ this->openInOption<TextureComponent>(component); }, component->frame);
//         mouseArea->setZ(component->pos.z + 1);
//         break;

//     case UiComponentType::TEXT:
//         // component = new Sentence({"Text"}, 2.0f, fontLoader);

//         // component->setTopAnchor(sceneEntityC->top);
//         // component->setLeftAnchor(sceneEntityC->left);

//         // component->setTopMargin(componentY - component->height / 2.0f);
//         // component->setLeftMargin(componentX - component->width / 2.0f);

//         // mouseArea = new Button(ecsRef, [=](Input*, double){ this->openInOption<Sentence>(component); }, component->frame);
//         // mouseArea->setZ(component->pos.z + 1);
//         break;

//     case UiComponentType::TEXTINPUT:
//         component = new TextInput(UiFrame{0.0f, 0.0f, 10.0f, 100.0f, 50.0f}, "TabTexture", fontLoader, [](const std::string& text){std::cout << text << std::endl;});

//         component->setTopAnchor(sceneEntityC->top);
//         component->setLeftAnchor(sceneEntityC->left);

//         component->setTopMargin(componentY - component->height / 2.0f);
//         component->setLeftMargin(componentX - component->width / 2.0f);

//         mouseArea = new Button(ecsRef, [=](Input*, double){ this->openInOption<Sentence>(component); }, component->frame);
//         mouseArea->setZ(component->pos.z - 1);
//         break;

//     case UiComponentType::LIST:

//         break;

//     case UiComponentType::PREFAB:

//         break;

//     default:

//         break;
//     }

//     // sceneEcs.attach<SceneElement>(ent, index, component, mouseArea);
//     index++;

//     // contextMenu->hide();
// }

// void EditorWindow::quit(Input* inputHandler, double...)
// {
//     if(inputHandler->isKeyPressed(Qt::Key_Escape))
//         emit quitApp();
// }