#include "contextmenu.h"

#include <string>

#include "ECS/entitysystem.h"
#include "Loaders/fontloader.h"
#include "UI/button.h"
#include "UI/texture.h"
#include "Renderer/renderer.h"
#include "Scene/scenemanager.h"

namespace pg
{
    namespace
    {
        static constexpr char const * DOM = "Context Menu";
    }
    
    template <>
    void renderer(MasterRenderer *masterRenderer, pg::editor::ContextMenu *contextMenu)
    {
        // contextMenu->backgroundTextureC->render(masterRenderer);

        // contextMenu->addButtonButtonC->render(masterRenderer);
        // contextMenu->addTextureButtonC->render(masterRenderer);
        // contextMenu->addTextButtonC->render(masterRenderer);
        // contextMenu->addTextInputButtonC->render(masterRenderer);
        // contextMenu->addListButtonC->render(masterRenderer);
    }

namespace editor
{
    ContextMenu::ContextMenu()
    {
        LOG_THIS_MEMBER(DOM);

        // this->width = 0;
        // this->height = 0;

        // [Start] Add Button

        // auto addButtonButton = ecs.createEntity();
        // addButtonButtonC = ecs.attach<Button>(addButtonButton,
        //     &ecs,
        //     [&](Input*, double){ this->callback(UiComponentType::BUTTON); },
        //     Sentence::SentenceParameters{{"Add Button"}, 2.0f, fontLoader}
        //     );

        // addButtonButtonC->setTopAnchor(this->top);
        // addButtonButtonC->setLeftAnchor(this->left);

        // addButtonButtonC->pos.z = this->pos.z;

        // this->width = addButtonButtonC->width > this->width ? addButtonButtonC->width : this->width;
        // this->height += addButtonButtonC->height;

        // // [End] Add Button

        // // [Start] Add Texture

        // auto addTextureButton = ecs.createEntity();
        // addTextureButtonC = ecs.attach<Button>(addTextureButton,
        //     &ecs,
        //     [&](Input*, double){ this->callback(UiComponentType::TEXTURE); },
        //     Sentence::SentenceParameters{{"Add Texture"}, 2.0f, fontLoader}
        //     );

        // addTextureButtonC->setTopAnchor(addButtonButtonC->bottom);
        // addTextureButtonC->setLeftAnchor(this->left);

        // addTextureButtonC->pos.z = this->pos.z;

        // this->width  = addTextureButtonC->width > this->width ? addTextureButtonC->width : this->width;
        // this->height += addTextureButtonC->height;

        // // [End] Add Texture

        // // [Start] Add Text

        // auto addTextButton = ecs.createEntity();
        // addTextButtonC = ecs.attach<Button>(addTextButton,
        //     &ecs,
        //     [&](Input*, double){ this->callback(UiComponentType::TEXT); },
        //     Sentence::SentenceParameters{{"Add Text"}, 2.0f, fontLoader}
        //     );

        // addTextButtonC->setTopAnchor(addTextureButtonC->bottom);
        // addTextButtonC->setLeftAnchor(this->left);

        // addTextButtonC->pos.z = this->pos.z;

        // this->width  = addTextButtonC->width > this->width ? addTextButtonC->width : this->width;
        // this->height += addTextButtonC->height;

        // // [End] Add Text

        // // [Start] Add Text

        // auto addTextInputButton = ecs.createEntity();
        // addTextInputButtonC = ecs.attach<Button>(addTextInputButton,
        //     &ecs,
        //     [&](Input*, double){ this->callback(UiComponentType::TEXTINPUT); },
        //     Sentence::SentenceParameters{{"Add Text Input"}, 2.0f, fontLoader}
        //     );

        // addTextInputButtonC->setTopAnchor(addTextButtonC->bottom);
        // addTextInputButtonC->setLeftAnchor(this->left);

        // addTextInputButtonC->pos.z = this->pos.z;

        // this->width  = addTextInputButtonC->width > this->width ? addTextInputButtonC->width : this->width;
        // this->height += addTextInputButtonC->height;

        // // [End] Add Text

        // // [Start] Add List

        // auto addListButton = ecs.createEntity();
        // addListButtonC = ecs.attach<Button>(addListButton,
        //     &ecs,
        //     [&](Input*, double){ this->callback(UiComponentType::LIST); },
        //     Sentence::SentenceParameters{{"Add List"}, 2.0f, fontLoader}
        //     );

        // addListButtonC->setTopAnchor(addTextInputButtonC->bottom);
        // addListButtonC->setLeftAnchor(this->left);

        // addListButtonC->pos.z = this->pos.z;

        // this->width  = addListButtonC->width > this->width ? addListButtonC->width : this->width;
        // this->height += addListButtonC->height;

        // // [End] Add List

        // // [Start] Add Prefab

        // auto addPrefabButton = ecs.createEntity();
        // addPrefabButtonC = ecs.attach<Button>(addPrefabButton,
        //     &ecs,
        //     [&](Input*, double){ this->callback(UiComponentType::LIST); },
        //     Sentence::SentenceParameters{{"Add Prefab"}, 2.0f, fontLoader}
        //     );

        // addPrefabButtonC->setTopAnchor(addListButtonC->bottom);
        // addPrefabButtonC->setLeftAnchor(this->left);

        // addPrefabButtonC->pos.z = this->pos.z;

        // this->width  = addPrefabButtonC->width > this->width ? addPrefabButtonC->width : this->width;
        // this->height += addPrefabButtonC->height;

        // [End] Add Prefab

        // [Start] Create background texture

        // auto backgroundTexture = ecs.createEntity();
        // backgroundTextureC = ecs.attach<TextureComponent>(backgroundTexture,
        //     this->width,
        //     this->height,
        //     textureName
        //     );

        // backgroundTextureC->setTopAnchor(this->top);
        // backgroundTextureC->setLeftAnchor(this->left);

        // backgroundTextureC->pos.z = this->pos.z;

        // [End] Create background texture

    }

    ContextMenu::~ContextMenu()
    {
        LOG_THIS_MEMBER(DOM);
    }

    void ContextMenu::init()
    {
        LOG_THIS_MEMBER(DOM);

        parent = ecsRef->createEntity();

        parentUi = ecsRef->attach<UiComponent>(parent);

        ecsRef->attach<MouseLeaveClickComponent>(parent, makeCallable<HideContextMenu>());

        auto backTexture = makeUiTexture(ecsRef, 1, 1, "TabTexture");

        auto background = backTexture.entity;

        backgroundC = backTexture.get<UiComponent>();

        backgroundC->fill(parentUi);
        
        // Todo move this in the ctor of the Context menu cause it is the only thing preventing this class to be generic
        setContextList("Add Sentence",  makeCallable<CreateElement>(UiComponentType::TEXT),
                       "Add Texture",   makeCallable<CreateElement>(UiComponentType::TEXTURE),
                       "Add Button",    makeCallable<CreateElement>(UiComponentType::BUTTON),
                       "Add TextInput", makeCallable<CreateElement>(UiComponentType::TEXTINPUT),
                       "Add List",      makeCallable<CreateElement>(UiComponentType::LIST),
                       "Add Prefab",    makeCallable<CreateElement>(UiComponentType::PREFAB));

        hide();

        parentUi->update();
    }

    void ContextMenu::setContextList(const std::string& text, CallablePtr callable)
    {
        addItemInContextMenu(text, callable);
    }

    void ContextMenu::addItemInContextMenu(const std::string& text, CallablePtr callable)
    {
        auto addItem = makeSentence(ecsRef, 0, 0, {text});

        auto addItemEntity = addItem.entity;

        ecsRef->attach<MouseLeftClickComponent>(addItemEntity, callable);

        auto addItemC = addItem.get<UiComponent>();

        if(components.size() > 0)
            addItemC->setTopAnchor(components.back()->bottom);
        else
            addItemC->setTopAnchor(parentUi->top);
        
        addItemC->setLeftAnchor(parentUi->left);

        if(addItemC->width > parentUi->width)
        {
            parentUi->setWidth(addItemC->width);

            for(auto comp : components)
            {
                comp->setWidth(addItemC->width);
            }
        }

        parentUi->height += addItemC->height;

        components.push_back(addItemC);
    }

    void ContextMenu::hide()
    {
        LOG_THIS_MEMBER(DOM);

        parentUi->hide();

        backgroundC->hide();

        for(auto comp : components)
        {
            comp->hide();
        }
    }

    void ContextMenu::onEvent(const ShowContextMenu& event)
    {
        LOG_THIS_MEMBER(DOM);

        LOG_MILE("Context Menu", "Show context");

        auto pos = event.inputHandler->getMousePos();

        parentUi->show();

        currentX = pos.x();
        currentY = pos.y();

        // Todo check for width / height overflow 

        parentUi->setX(pos.x());
        parentUi->setY(pos.y());

        backgroundC->show();
        
        for(auto comp : components)
        {
            comp->show();
        }
    }

    void ContextMenu::onEvent(const HideContextMenu&)
    {
        LOG_THIS_MEMBER(DOM);

        hide();
    }

    void ContextMenu::onEvent(const CreateElement& event)
    {
        switch(event.type)
        {
            case UiComponentType::TEXT:
                {
                    auto newElement = makeSentence(ecsRef, currentX, currentY, {"New Text"});

                    ecsRef->attach<SceneElement>(newElement.entity);
                }
                break;

            case UiComponentType::TEXTURE:

                break;

            default:
                break;
        }

        hide();
    }

}
}