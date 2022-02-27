#pragma once

#include <vector>

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>

#include "../../constant.h"

#include "../../Engine/basesystem.h"
#include "../../Engine/tileloader.h"
#include "../../Engine/fontloader.h"
#include "../../Engine/map.h"
#include "../../UI/uisystem.h"
#include "../../UI/sentencesystem.h"
#include "../../Input/input.h"
#include "../../Input/inputcomponent.h"

using namespace pg;

class TileSelector : public UiComponent, protected QOpenGLFunctions
{
public:
    TileSelector(Map *map, TilesLoader *TilesLoader, FontLoader *fontLoader, UiComponent *screenUi);

    void mouseInput(Input* inputHandler, double deltaTime);

    void setVisibility(bool visibility);
    inline bool isVisible() const { return visible; }

    virtual void render(MasterRenderer* masterRenderer) { renderer(masterRenderer, this); }

    ~TileSelector();

private:
    friend void renderer<>(MasterRenderer* masterRenderer, TileSelector* tileSelector);
    bool visible = false;

    TextureComponent *texture;

    std::vector<LoaderRenderComponent<TilesLoader::TilesId>> tileRendererVector;
    std::vector<MouseInputComponent* > mouseAreaVector;

    std::vector<Sentence*> textVector;

    Map *map;
    TilesLoader *tileLoader;
    FontLoader *fontLoader;
};