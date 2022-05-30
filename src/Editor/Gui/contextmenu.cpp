#include "contextmenu.h"

#include <string>
#include <functional>

#include "ECS/entitysystem.h"
#include "Loaders/fontloader.h"
#include "UI/button.h"
#include "Renderer/renderer.h"

namespace pg
{
    template<>
    void renderer(MasterRenderer *masterRenderer, pg::editor::ContextMenu *contextMenu)
    {
        contextMenu->backgroundTextureC->render(masterRenderer);

        contextMenu->addButtonButtonC->render(masterRenderer);
        contextMenu->addTextureButtonC->render(masterRenderer);
        contextMenu->addTextButtonC->render(masterRenderer);
        contextMenu->addListButtonC->render(masterRenderer);
    }

namespace editor
{
    ContextMenu::ContextMenu(EntitySystem &ecs, FontLoader *fontLoader, const std::string& textureName, const std::function<void(const UiComponentType&)>& callback) : UiComponent()
    {
        this->width = 0;
        this->height = 0;

        // [Start] Add Button

        auto addButtonButton = ecs.createEntity();
        addButtonButtonC = ecs.attach<Button>(addButtonButton,
            [&](Input*, double){ callback(UiComponentType::BUTTON); },
            Sentence::SentenceParameters{{"Add Button"}, 2.0f, fontLoader}
            );

        addButtonButtonC->setTopAnchor(this->top);
        addButtonButtonC->setLeftAnchor(this->left);

        addButtonButtonC->pos.z = this->pos.z;

        this->width = addButtonButtonC->width > this->width ? addButtonButtonC->width : this->width;

        // [End] Add Button

        // [Start] Add Texture

        auto addTextureButton = ecs.createEntity();
        addTextureButtonC = ecs.attach<Button>(addTextureButton,
            [&](Input*, double){ callback(UiComponentType::TEXTURE); },
            Sentence::SentenceParameters{{"Add Texture"}, 2.0f, fontLoader}
            );

        addTextureButtonC->setTopAnchor(addButtonButtonC->bottom);
        addTextureButtonC->setLeftAnchor(this->left);

        addTextureButtonC->pos.z = this->pos.z;

        this->width  = addTextureButtonC->width > this->width ? addTextureButtonC->width : this->width;

        // [End] Add Texture

        // [Start] Add Text

        auto addTextButton = ecs.createEntity();
        addTextButtonC = ecs.attach<Button>(addTextButton,
            [&](Input*, double){ callback(UiComponentType::TEXT); },
            Sentence::SentenceParameters{{"Add Text"}, 2.0f, fontLoader}
            );

        addTextButtonC->setTopAnchor(addTextureButtonC->bottom);
        addTextButtonC->setLeftAnchor(this->left);

        addTextButtonC->pos.z = this->pos.z;

        this->width  = addTextButtonC->width > this->width ? addTextButtonC->width : this->width;

        // [End] Add Text

        // [Start] Add Text

        auto addListButton = ecs.createEntity();
        addListButtonC = ecs.attach<Button>(addListButton,
            [&](Input*, double){ callback(UiComponentType::LIST); },
            Sentence::SentenceParameters{{"Add List"}, 2.0f, fontLoader}
            );

        addListButtonC->setTopAnchor(addTextButtonC->bottom);
        addListButtonC->setLeftAnchor(this->left);

        addListButtonC->pos.z = this->pos.z;

        this->width  = addListButtonC->width > this->width ? addListButtonC->width : this->width;

        // [End] Add Text

        // [Start] Create background texture

        auto backgroundTexture = ecs.createEntity();
        backgroundTextureC = ecs.attach<TextureComponent>(backgroundTexture,
            this->width,
            this->height,
            textureName
            );

        backgroundTextureC->setTopAnchor(this->top);
        backgroundTextureC->setLeftAnchor(this->left);

        backgroundTextureC->pos.z = this->pos.z;

        // [End] Create background texture

    }

    ContextMenu::~ContextMenu()
    {

    }

    void ContextMenu::render(MasterRenderer* masterRenderer)
    {
        renderer(masterRenderer, this);
    }

    void ContextMenu::show()
    {
        backgroundTextureC->show();

        addButtonButtonC->show();
        addTextureButtonC->show();
        addTextButtonC->show();
        addListButtonC->show();
    }

    void ContextMenu::hide()
    {
        backgroundTextureC->hide();

        addButtonButtonC->hide();
        addTextureButtonC->hide();
        addTextButtonC->hide();
        addListButtonC->hide();
    }
}
}