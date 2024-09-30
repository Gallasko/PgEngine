#include "listview.h"

#include "scrollable.h"

#include "2D/texture.h"

#include "focusable.h"

#include "Systems/oneventcomponent.h"

#include "Input/inputcomponent.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <SDL2/SDL.h>
#include <SDL_opengles2.h>
// #include <SDL_opengl_glext.h>
// #include <GLES2/gl2.h>
// #include <GLFW/glfw3.h>
#else
    #ifdef __linux__
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_opengl.h>
    #elif _WIN32
    #include <SDL.h>
    #include <SDL_opengl.h>
    #endif
#endif

namespace pg
{
    namespace
    {
        static constexpr char const * DOM = "List view";
    }

    void ListView::removeEntity(EntityRef /* entity */)
    {

    }

    void ListView::updateVisibility()
    {
        for (auto& ui : entities)
        {
            float childTop    = ui->top;
            float childBottom = ui->bottom;
            float childLeft   = ui->left;
            float childRight  = ui->right;

            ui->visible = false;

            if (viewUi->inBound(childLeft, childTop) or viewUi->inBound(childLeft, childBottom) or viewUi->inBound(childRight, childTop) or viewUi->inBound(childRight, childBottom))
            {
                ui->visible = true;
            }

            ui->update();
        }
    }

    void ListViewSystem::init()
    {
        
    }

    void ListViewSystem::addEntity(CompRef<ListView> view, CompRef<UiComponent> ui)
    {
        auto& viewUi = view->viewUi;

        if (view->entities.size() > 0)
        {
            ui->setTopAnchor(view->entities.back()->bottom);

            ui->setTopMargin(view->spacing);
        }
        else
        {
            ui->setTopAnchor(viewUi->top);
        }

        ui->setLeftAnchor(viewUi->left);

        // Z + 1 so the initial z of the list is for the background
        ui->setZ(viewUi->pos.z + 1);

        ui->setClipRect({viewUi->top, viewUi->left}, {viewUi->bottom, viewUi->right});

        view->entities.push_back(ui);

        calculateListSize(view);

        view->updateVisibility();
    }

    void ListViewSystem::calculateListSize(CompRef<ListView> view)
    {
        view->listReelHeight = 0;

        for (auto ui : view->entities)
        {
            view->listReelHeight += ui->height + view->spacing;
        }

        updateCursorSize(view, view->listReelHeight);
    }

    void ListViewSystem::updateCursorSize(CompRef<ListView> view, const UiSize& maxPos)
    {
        auto height = view->viewUi->height;

        if (maxPos > 0 && height > 0)
        {
            view->cursorHeight = (height / maxPos) * height;
            
            if (view->cursorHeight > height)
                view->cursorHeight = height;
        }
        else
        {
            view->cursorHeight = height;
        }

        view->cursor.get<UiComponent>()->setHeight(view->cursorHeight);
    }

    CompList<UiComponent, ListView> makeListView(EntitySystem *ecs, float x, float y, float width, float height)
    {
        auto entity = ecs->createEntity();

        auto ui = ecs->attach<UiComponent>(entity);

        // auto scrollable = ecs->attach<Scrollable>(entity);

        // scrollable->horizontalSlider = ecs->createEntity();
        // scrollable->horizontalCursor = ecs->createEntity();
        // scrollable->verticalSlider = ecs->createEntity();
        // scrollable->verticalCursor = ecs->createEntity();

        auto view = ecs->attach<ListView>(entity);

        // make2

        ui->setX(x);
        ui->setY(y);

        ui->setWidth(width);
        ui->setHeight(height);

        view->viewUi = ui;

        // Z + 3 so the cursor is always on top of the slider
        auto cursor = makeUiTexture(ecs, 15, 40, "cursor");
        cursor.get<UiComponent>()->setZ(ui->pos.z + 3);
        cursor.get<UiComponent>()->setHeight(view->cursorHeight);
        cursor.get<UiComponent>()->setTopAnchor(ui->top);
        cursor.get<UiComponent>()->setRightAnchor(ui->right);

        ecs->attach<FocusableComponent>(cursor.entity);

        auto viewId = view.entityId;

        auto cursorId = cursor.entity.id;

        // Todo improve this
        std::function<void(const OnMouseMove&)> cursorCallback = [viewId, ecs](const OnMouseMove event) {
            if (not event.inputHandler->isButtonPressed(SDL_BUTTON_LEFT))
                return;

            auto ent = ecs->getEntity(viewId);

            if (not ent)
                return;

            auto viewComp = ent->get<ListView>();
            auto viewUi = ent->get<UiComponent>();

            auto focus = viewComp->cursor->get<FocusableComponent>();
            auto cursorUi = viewComp->cursor->get<UiComponent>();

            if (not focus->focused)
                return;

            float cHeight = viewComp->cursorHeight;

            float currentPos = event.pos.y - viewUi->pos.y - cHeight / 2.0f;

            if (currentPos < 0)
                currentPos = 0;

            float maxHeight = viewUi->height - cursorUi->height;

            if (currentPos > maxHeight)
                currentPos = maxHeight;

            cursorUi->setTopMargin(currentPos);

            if (viewComp->entities.size() > 0)
            {
                // * static_cast<float>(viewComp->listReelHeight / viewComp->viewUi->height)
                viewComp->entities[0]->setTopMargin(-currentPos * static_cast<float>(viewComp->listReelHeight / viewComp->viewUi->height));
            }

            viewComp->updateVisibility();
        };

        ecs->attach<OnEventComponent>(cursor.entity, cursorCallback);

        ecs->attach<MouseLeftClickComponent>(cursor.entity, makeCallable<OnFocus>(cursorId), MouseStateTrigger::OnPress);

        // Z + 2 so the slider is always on top of any entity in the list
        auto slider = makeUiTexture(ecs, 15, 1, "slider");
        slider.get<UiComponent>()->setZ(ui->pos.z + 2);
        slider.get<UiComponent>()->setTopAnchor(ui->top);
        slider.get<UiComponent>()->setBottomAnchor(ui->bottom);
        slider.get<UiComponent>()->setRightAnchor(ui->right);

        view->cursor = cursor.entity;
        view->slider = slider.entity;

        return CompList<UiComponent, ListView>(entity, ui, view);
    }

    void ListViewSystem::clear(CompRef<ListView> view)
    {
        for (auto ent : view->entities)
        {
            ecsRef->removeEntity(ent.entityId);
        }

        view->entities.clear();

        view->cursor.get<UiComponent>()->setTopMargin(0);
    }
}