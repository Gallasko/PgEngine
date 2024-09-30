#pragma once

#include "Scene/scenemanager.h"

#include "tetromino.h"

using namespace pg;

struct PlayClicked {};
struct OptionClicked {};

#include "2D/animator2d.h"
#include "2D/texture.h"

struct TitleScreen : public Scene
{
    virtual void init() override;

    virtual void startUp() override { startedUp = true; }

    bool startedUp = false;

    CompRef<UiComponent> logoText[5];

    bool startTransi = false;

    CompRef<Texture2DAnimationComponent> transi;
};