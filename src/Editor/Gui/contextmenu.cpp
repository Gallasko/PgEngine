#include "contextmenu.h"

#include <string>

#include "ECS/entitysystem.h"
#include "Loaders/fontloader.h"
#include "UI/uisystem.h"
#include "UI/sentencesystem.h"

// #include "Input/inputcomponent.h"
#include "2D/texture.h"
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

        parentUi->setZ(10);

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

        addItemC->setZ(11);

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

        currentX = pos.x;
        currentY = pos.y;

        // Todo check for width / height overflow 

        parentUi->setX(pos.x);
        parentUi->setY(pos.y);

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
                {
                    auto newElement = makeUiTexture(ecsRef, 50, 50, "TabTexture");

                    auto uiComp = newElement.get<UiComponent>();

                    uiComp->setX(currentX);
                    uiComp->setY(currentY);

                    ecsRef->attach<SceneElement>(newElement.entity);
                }
                break;

            default:
                break;
        }

        hide();
    }

}
}