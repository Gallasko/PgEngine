#pragma once

#include <vector>

#include "../../Engine/constant.h"

#include "../../Engine/Loaders/tileloader.h"
#include "../../Engine/Loaders/fontloader.h"
#include "../Systems/map.h"
#include "../../Engine/UI/uisystem.h"
#include "../../Engine/UI/sentencesystem.h"
#include "../../Engine/Input/input.h"
#include "../../Engine/Input/inputcomponent.h"

using namespace pg;

namespace pg
{
    class TextureComponent;
}

class TileSelector : public UiComponent
{
public:
    TileSelector(Map *map, TilesLoader *TilesLoader, FontLoader *fontLoader, UiComponent *screenUi);
    ~TileSelector();

    void setVisibility(bool visibility);
    inline bool isVisible() const { return visible; }

    virtual void render(MasterRenderer* masterRenderer);

private:
    friend void renderer<>(MasterRenderer* masterRenderer, TileSelector* tileSelector);
    bool visible = false;

    TextureComponent *texture;

    std::vector<LoaderRenderComponent<TilesLoader::TilesId>> tileRendererVector;
    std::vector<MouseInput> mouseAreaVector;

    std::vector<Sentence*> textVector;

    Map *map;
    TilesLoader *tileLoader;
    FontLoader *fontLoader;
};