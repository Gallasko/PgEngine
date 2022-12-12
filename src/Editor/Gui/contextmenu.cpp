#include "contextmenu.h"

#include <string>

#include "ECS/entitysystem.h"
#include "Loaders/fontloader.h"
#include "UI/button.h"
#include "UI/texture.h"
#include "Renderer/renderer.h"

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

        contextMenu->addButtonButtonC->render(masterRenderer);
        contextMenu->addTextureButtonC->render(masterRenderer);
        contextMenu->addTextButtonC->render(masterRenderer);
        contextMenu->addTextInputButtonC->render(masterRenderer);
        contextMenu->addListButtonC->render(masterRenderer);
    }

namespace editor
{
    ContextMenu::ContextMenu(EntitySystem &ecs, FontLoader *fontLoader, const std::string& textureName, const std::function<void(const UiComponentType&)>& callback) : UiComponent(), callback(callback)
    {
        this->width = 0;
        this->height = 0;

        // [Start] Add Button

        auto addButtonButton = ecs.createEntity();
        addButtonButtonC = ecs.attach<Button>(addButtonButton,
            [&](Input*, double){ this->callback(UiComponentType::BUTTON); },
            Sentence::SentenceParameters{{"Add Button"}, 2.0f, fontLoader}
            );

        addButtonButtonC->setTopAnchor(this->top);
        addButtonButtonC->setLeftAnchor(this->left);

        addButtonButtonC->pos.z = this->pos.z;

        this->width = addButtonButtonC->width > this->width ? addButtonButtonC->width : this->width;
        this->height += addButtonButtonC->height;

        // [End] Add Button

        // [Start] Add Texture

        auto addTextureButton = ecs.createEntity();
        addTextureButtonC = ecs.attach<Button>(addTextureButton,
            [&](Input*, double){ this->callback(UiComponentType::TEXTURE); },
            Sentence::SentenceParameters{{"Add Texture"}, 2.0f, fontLoader}
            );

        addTextureButtonC->setTopAnchor(addButtonButtonC->bottom);
        addTextureButtonC->setLeftAnchor(this->left);

        addTextureButtonC->pos.z = this->pos.z;

        this->width  = addTextureButtonC->width > this->width ? addTextureButtonC->width : this->width;
        this->height += addTextureButtonC->height;

        // [End] Add Texture

        // [Start] Add Text

        auto addTextButton = ecs.createEntity();
        addTextButtonC = ecs.attach<Button>(addTextButton,
            [&](Input*, double){ this->callback(UiComponentType::TEXT); },
            Sentence::SentenceParameters{{"Add Text"}, 2.0f, fontLoader}
            );

        addTextButtonC->setTopAnchor(addTextureButtonC->bottom);
        addTextButtonC->setLeftAnchor(this->left);

        addTextButtonC->pos.z = this->pos.z;

        this->width  = addTextButtonC->width > this->width ? addTextButtonC->width : this->width;
        this->height += addTextButtonC->height;

        // [End] Add Text

        // [Start] Add Text

        auto addTextInputButton = ecs.createEntity();
        addTextInputButtonC = ecs.attach<Button>(addTextInputButton,
            [&](Input*, double){ this->callback(UiComponentType::TEXTINPUT); },
            Sentence::SentenceParameters{{"Add Text Input"}, 2.0f, fontLoader}
            );

        addTextInputButtonC->setTopAnchor(addTextButtonC->bottom);
        addTextInputButtonC->setLeftAnchor(this->left);

        addTextInputButtonC->pos.z = this->pos.z;

        this->width  = addTextInputButtonC->width > this->width ? addTextInputButtonC->width : this->width;
        this->height += addTextInputButtonC->height;

        // [End] Add Text

        // [Start] Add List

        auto addListButton = ecs.createEntity();
        addListButtonC = ecs.attach<Button>(addListButton,
            [&](Input*, double){ this->callback(UiComponentType::LIST); },
            Sentence::SentenceParameters{{"Add List"}, 2.0f, fontLoader}
            );

        addListButtonC->setTopAnchor(addTextInputButtonC->bottom);
        addListButtonC->setLeftAnchor(this->left);

        addListButtonC->pos.z = this->pos.z;

        this->width  = addListButtonC->width > this->width ? addListButtonC->width : this->width;
        this->height += addListButtonC->height;

        // [End] Add List

        // [Start] Add Prefab

        auto addPrefabButton = ecs.createEntity();
        addPrefabButtonC = ecs.attach<Button>(addPrefabButton,
            [&](Input*, double){ this->callback(UiComponentType::LIST); },
            Sentence::SentenceParameters{{"Add Prefab"}, 2.0f, fontLoader}
            );

        addPrefabButtonC->setTopAnchor(addListButtonC->bottom);
        addPrefabButtonC->setLeftAnchor(this->left);

        addPrefabButtonC->pos.z = this->pos.z;

        this->width  = addPrefabButtonC->width > this->width ? addPrefabButtonC->width : this->width;
        this->height += addPrefabButtonC->height;

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

    }

    void ContextMenu::render(MasterRenderer* masterRenderer)
    {
        renderer(masterRenderer, this);
    }

    void ContextMenu::show()
    {
        UiComponent::show();

        // backgroundTextureC->show();

        addButtonButtonC->show();
        addTextureButtonC->show();
        addTextButtonC->show();
        addTextInputButtonC->show();
        addListButtonC->show();
        addPrefabButtonC->show();
    }

    void ContextMenu::hide()
    {
        UiComponent::hide();

        // backgroundTextureC->hide();

        addButtonButtonC->hide();
        addTextureButtonC->hide();
        addTextButtonC->hide();
        addTextInputButtonC->hide();
        addListButtonC->hide();
        addPrefabButtonC->hide();
    }
}
}