#include "titlescreen.h"

#include "ECS/system.h"
#include "Input/inputcomponent.h"

#include "Systems/coresystems.h"

#include "keyconfig.h"

#include "tetromino.h"
#include "2D/texture.h"

#include "game.h"

struct GotoGameEvent {};

void TitleScreen::init()
{
    LOG_INFO("TitleScreen", "Loading title...");

    auto back = makeUiTexture(this, 820, 640, "back");

    auto versionText = makeSentence(this, 0, 0, {"Version 1.1"});

    versionText.get<UiComponent>()->setLeftAnchor(back.get<UiComponent>()->left);
    versionText.get<UiComponent>()->setBottomAnchor(back.get<UiComponent>()->bottom);

    versionText.get<UiComponent>()->setRightMargin(5);
    versionText.get<UiComponent>()->setBottomMargin(5);
    versionText.get<UiComponent>()->setZ(1);

    auto usingEngine = makeSentence(this, 0, 0, {"Using PG Engine"});

    usingEngine.get<UiComponent>()->setRightAnchor(back.get<UiComponent>()->right);
    usingEngine.get<UiComponent>()->setBottomAnchor(back.get<UiComponent>()->bottom);

    usingEngine.get<UiComponent>()->setRightMargin(5);
    usingEngine.get<UiComponent>()->setBottomMargin(5);
    usingEngine.get<UiComponent>()->setZ(1);

    auto madeBy = makeSentence(this, 0, 0, {"Made by Pigeon Codeur"});

    madeBy.get<UiComponent>()->setRightAnchor(back.get<UiComponent>()->right);
    madeBy.get<UiComponent>()->setBottomAnchor(usingEngine.get<UiComponent>()->top);

    madeBy.get<UiComponent>()->setRightMargin(5);
    madeBy.get<UiComponent>()->setBottomMargin(15);

    madeBy.get<UiComponent>()->setZ(1);

    auto t = makeUiTexture(this, 27 * 3, 38 * 3, "ui.t");

    t.get<UiComponent>()->setX(80);
    t.get<UiComponent>()->setY(80);
    t.get<UiComponent>()->setZ(5);

    logoText[0] = t.get<UiComponent>();

    auto e = makeUiTexture(this, 28 * 3, 38 * 3, "ui.e");

    e.get<UiComponent>()->setLeftAnchor(t.get<UiComponent>()->right);
    e.get<UiComponent>()->setLeftMargin(12);
    e.get<UiComponent>()->setY(80);
    e.get<UiComponent>()->setZ(5);

    logoText[1] = e.get<UiComponent>();

    auto p = makeUiTexture(this, 20 * 3, 38 * 3, "ui.p");

    p.get<UiComponent>()->setLeftAnchor(e.get<UiComponent>()->right);
    p.get<UiComponent>()->setLeftMargin(12);
    p.get<UiComponent>()->setY(80);
    p.get<UiComponent>()->setZ(5);

    logoText[2] = p.get<UiComponent>();

    auto l = makeUiTexture(this, 19 * 3, 38 * 3, "ui.l");

    l.get<UiComponent>()->setLeftAnchor(p.get<UiComponent>()->right);
    l.get<UiComponent>()->setLeftMargin(12);
    l.get<UiComponent>()->setY(80);
    l.get<UiComponent>()->setZ(5);

    logoText[3] = l.get<UiComponent>();

    auto a = makeUiTexture(this, 26 * 3, 38 * 3, "ui.a");

    a.get<UiComponent>()->setLeftAnchor(l.get<UiComponent>()->right);
    a.get<UiComponent>()->setLeftMargin(12);
    a.get<UiComponent>()->setY(80);
    a.get<UiComponent>()->setZ(5);

    logoText[4] = a.get<UiComponent>();

    auto red = makeUiTexture(this, 48, 48, "tiles.blank");

    red.get<UiComponent>()->setX(470);
    red.get<UiComponent>()->setY(290);
    red.get<UiComponent>()->setZ(5);

    red.get<Texture2DComponent>()->setOverlappingColor({5.6, 0, 0}, 0.1);

    auto redSent = makeSentence(this, 525, 305, {": +10\% DAMAGE"});
    redSent.get<UiComponent>()->setZ(5);

    auto green = makeUiTexture(this, 48, 48, "tiles.blank");

    green.get<UiComponent>()->setX(470);
    green.get<UiComponent>()->setY(350);
    green.get<UiComponent>()->setZ(5);

    green.get<Texture2DComponent>()->setOverlappingColor({0, 13.5, 0}, 0.05);

    auto greenSent = makeSentence(this, 525, 365, {": +20\% HEALTH"});
    greenSent.get<UiComponent>()->setZ(5);

    auto blue = makeUiTexture(this, 48, 48, "tiles.blank");

    blue.get<UiComponent>()->setX(470);
    blue.get<UiComponent>()->setY(410);
    blue.get<UiComponent>()->setZ(5);

    blue.get<Texture2DComponent>()->setOverlappingColor({0, 0, 7.1}, 0.1);

    auto blueSent = makeSentence(this, 525, 425, {": +10\% RANGE"});
    blueSent.get<UiComponent>()->setZ(5);

    auto purple = makeUiTexture(this, 48, 48, "tiles.blank");

    purple.get<UiComponent>()->setX(470);
    purple.get<UiComponent>()->setY(470);
    purple.get<UiComponent>()->setZ(5);

    purple.get<Texture2DComponent>()->setOverlappingColor({1.5,0.2,1.5}, 0.4);

    auto purpleSent = makeSentence(this, 525, 485, {": +10\% FIRERATE"});
    purpleSent.get<UiComponent>()->setZ(5);

    auto playButton = makeUiTexture(this, 43 * 4, 24 * 4, "ui.start0");

    playButton.get<UiComponent>()->setX(80);
    playButton.get<UiComponent>()->setY(270);

    attach<MouseLeftClickComponent>(playButton.entity, makeCallable<PlayClicked>());
    attach<Texture2DAnimationComponent>(playButton.entity, Texture2DAnimationComponent{ {{0, Texture2DComponent{"ui.start1"}}, {100, Texture2DComponent{"ui.start1"}}, {200, Texture2DComponent{"ui.start2"}}, {300, Texture2DComponent{"ui.start1"}},}, false });

    auto transiTex = makeUiTexture(this, 820, 640, "transi.0");
    transiTex.get<UiComponent>()->setZ(100);

    std::vector<Animation2DKeyPoint> keypoints;

    auto callback = makeCallable<GotoGameEvent>();

    for (int i = 0; i < 9; i++)
    {
        keypoints.emplace_back(i * 50, "transi." + std::to_string(i), i == 8 ? callback : nullptr);
    }

    transi = attach<Texture2DAnimationComponent>(transiTex.entity, Texture2DAnimationComponent{keypoints, false});

    listenToEvent<GotoGameEvent>([this](const GotoGameEvent&) { ecsRef->getSystem<SceneElementSystem>()->loadSystemScene<GameScene>(); });

    listenToEvent<PlayClicked>([this](const PlayClicked&) { transi->start(); });

    listenToEvent<TickEvent>([this](const TickEvent& event) {
        if (not startedUp)
            return; 
        
        static auto elapsedTime = 0;

        static bool stepCompleted[10] = {false};

        elapsedTime += event.tick;

        for (int i = 0; i < 5; i++)
        {
            if (elapsedTime > i * 200 + 100 and not stepCompleted[i])
            {
                logoText[i]->setY(70);

                stepCompleted[i] = true;
            }

            if (elapsedTime > i * 200 + 1500 and not stepCompleted[i + 5])
            {
                logoText[i]->setY(80);

                stepCompleted[i + 5] = true;
            }
        }

        if (elapsedTime >= 2600)
        {
            for (size_t i = 0; i < 10; i++)
            {
                stepCompleted[i] = false;
            }

            elapsedTime -= 2600;
        }        
    });

    LOG_INFO("TitleScreen", "Title loaded");
}