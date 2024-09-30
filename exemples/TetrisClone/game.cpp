#include "game.h"

#include "2D/texture.h"
#include "2D/collisionsystem.h"

#include "Systems/coresystems.h"

#include "Systems/oneventcomponent.h"

#include "titlescreen.h"

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

#include "Audio/audiosystem.h"

namespace
{
    static const char* const DOM = "Game Scene";

    constexpr size_t BOARDOFFSET = 480;
    constexpr size_t REALTILESIZE = 16;
    size_t SCALE = 6; // Starting scale
    size_t TILESIZE = REALTILESIZE * SCALE;

    constexpr size_t TRUESCALE = 4; // True scale
    constexpr size_t TRUETILESIZE = REALTILESIZE * TRUESCALE;
}

struct StartingTileMovedEvent
{
    StartingTileMovedEvent(size_t x, size_t y) : x(x), y(y) {}
    StartingTileMovedEvent(const StartingTileMovedEvent& other) : x(other.x), y(other.y) {}

    size_t x;
    size_t y;
};

struct EnemyHit {};
struct TowerSelected
{
    TowerSelected(size_t tower) : tower(tower) {}
    TowerSelected(const TowerSelected& other) : tower(other.tower) {}
    size_t tower = 0;
};

#include <random>

static std::vector<TowerLayout> towerLayout = {
    {{{0, 0, 11},}},
    {{{0, 0, 11},}},
    {{{0, 0, 11},}},
    {{{0, 0, 11},}},
    {{{0, 0, 11},}},

    {{{0, 0, 0},  { 1,  0,  2}}},
    {{{0, 0, 4},  { 0,  1,  6}}},

    // Duplicate for more chances
    {{{0, 0, 0},  { 1,  0,  2}}},
    {{{0, 0, 4},  { 0,  1,  6}}},
    // Duplicate for more chances
    {{{0, 0, 0},  { 1,  0,  2}}},
    {{{0, 0, 4},  { 0,  1,  6}}},

    {{{0, 0, 1},  {-1,  0,  0}, { 1,  0,  2}}},
    {{{0, 0, 5},  { 0, -1,  4}, { 0,  1,  6}}},

    {{{0, 0, 7},  { 0, -1,  4}, { 1,  0,  2}}},
    {{{0, 0, 8},  {-1,  0,  0}, { 0,  1,  6}}},
    {{{0, 0, 9},  { 1,  0,  2}, { 0,  1,  6}}},
    {{{0, 0, 10}, {-1,  0,  0}, { 0, -1,  4}}},

    {{{0, 0, 7},  { 0, -1,  9}, { 1, -1,  8}, { 1,  0, 10}}},
    {{{0, 0, 3},  {-1,  0,  0}, { 0, -1,  4}, { 0,  1,  6}, { 1,  0,  2}}},
};

void colorBlock(CompRef<Texture2DComponent> comp, const TowerData& towerData)
{
    if (towerData.color == TowerColor::Red)
        comp->setOverlappingColor({5.6, 0, 0}, 0.1);
    else if (towerData.color == TowerColor::Green)
        comp->setOverlappingColor({0, 5.5, 0}, 0.05);
    else if (towerData.color == TowerColor::Blue)
        comp->setOverlappingColor({0, 0, 4.1}, 0.1);
    else if (towerData.color == TowerColor::Purple)
        comp->setOverlappingColor({1.5, 0.2, 1.5}, 0.4);
}

template <typename EntityRefLike>
void colorBlock(const EntityRefLike& entity, const TowerData& towerData)
{
    colorBlock(entity.template get<Texture2DComponent>(), towerData);
}

double randomNumber()
{
    // Making rng static ensures that it stays the same
    // Between different invocations of the function
    static std::default_random_engine rng(time(0));

    std::uniform_real_distribution<double> dist(0.0, 1.0); 
    return dist(rng); 
}

float posToTileMap(float pos)
{
    if (BOARDOFFSET + pos < 0)
    {
        return 0;
    }

    return (BOARDOFFSET + pos) / TILESIZE;
}

float tileMapToPos(float mapPos)
{
    return mapPos * TILESIZE - BOARDOFFSET;
}

void TowerCell::registerTower(TowerData data)
{
    bulletPrototype.damage += data.damage;
    bulletPrototype.damage *= data.damageMultiplier;

    range *= data.rangeMultiplier;
    shootInterval *= data.shootIntervalMultiplier;

    generalHealthMultiplier *= data.healthMultiplier;

    data.health *= generalHealthMultiplier;

    towerData.push_back(data);
}

void GameScene::init()
{
    generateTowerData();

    for (size_t i = 0; i < 50; i++)
    {
        for (size_t j = 0; j < 50; j ++)
        {
            size_t posX = tileMapToPos(i);
            size_t posY = tileMapToPos(j);

            std::string tileName = ((i + j) % 2) == 0 ? "tiles.light" : "tiles.dark"; 

            auto boardTex = makeUiTexture(this, TILESIZE, TILESIZE, tileName);

            boardTex.get<UiComponent>()->setX(posX);
            boardTex.get<UiComponent>()->setY(posY);
            boardTex.get<UiComponent>()->setZ(0);

            board[i][j] = boardTex.entity;

            towerBoard[i][j].x = i;
            towerBoard[i][j].y = j;
        }
    }

    auto timerTextUi = makeSentence(this, 10, 0, {"00:00", 4.0f, {255, 255, 255, 255}});

    timerTextUi.get<UiComponent>()->setZ(120);

    timerText = timerTextUi.get<SentenceText>();

    auto shopBack = makeUiTexture(this, 820, 400, "back");

    shopBack.get<UiComponent>()->setX(0);
    shopBack.get<UiComponent>()->setY(480);
    shopBack.get<UiComponent>()->setZ(120);

    for (size_t i = 0; i < 5; i++)
    {
        auto selected = makeUiTexture(this, 160, 160, "ui.shopSelection0");

        selected.get<UiComponent>()->setX(i *  160);
        selected.get<UiComponent>()->setTopAnchor(shopBack.get<UiComponent>()->top);
        selected.get<UiComponent>()->setZ(122);

        if (i == 0)
            selected.get<UiComponent>()->setVisibility(true); 
        else
            selected.get<UiComponent>()->setVisibility(false);
        
        shopSelected[i] = selected.entity;

        for (size_t y = 0; y < 5; y++)
        {
            for (size_t x = 0; x < 5; x++)
            {
                auto shopItem = makeUiTexture(this, 2 * REALTILESIZE, 2 * REALTILESIZE, "tiles.empty");

                shopItem.get<UiComponent>()->setTopAnchor(selected.get<UiComponent>()->top);
                shopItem.get<UiComponent>()->setLeftAnchor(selected.get<UiComponent>()->left);
                shopItem.get<UiComponent>()->setTopMargin(y * 2 * REALTILESIZE);
                shopItem.get<UiComponent>()->setLeftMargin(x * 2 * REALTILESIZE);
                shopItem.get<UiComponent>()->setZ(121);

                shopItemHolder[i][x][y] = shopItem.get<Texture2DComponent>();
            }
        }
    }

    for (size_t y = 0; y < 5; y++)
    {
        for (size_t x = 0; x < 5; x++)
        {
            auto previewItem = makeUiTexture(this, TRUETILESIZE, TRUETILESIZE, "tiles.empty");

            previewItem.get<Texture2DComponent>()->setOpacity(0.8);
            
            previewItem.get<UiComponent>()->setZ(3);

            preview[x][y] = previewItem.entity;
        }
    }

    auto obscUi = makeUiTexture(this, 820, 640, "ui.obsc");

    obsc = obscUi.get<UiComponent>();

    obsc->setZ(149);

    auto surviveUi0 = makeUiTexture(this, 820, 640, "doodles");
    surviveUi0.get<UiComponent>()->setZ(150);
    surviveUi0.get<UiComponent>()->setVisibility(false);

    auto surviveUi = makeSentence(this, 50, 55, {"SURVIVE AT LEAST"});
    surviveUi.get<UiComponent>()->setZ(151);

    auto surviveUi2 = makeSentence(this, 125, 85, {"10 MIN", 3.00f, {255, 0, 0, 255}});
    surviveUi2.get<UiComponent>()->setZ(151);

    auto surviveUi3 = makeSentence(this, 335, 290, {"PROTECT", 3.0f, {255, 0, 0, 255}});
    surviveUi3.get<UiComponent>()->setZ(151);

    auto surviveUi4 = makeSentence(this, 470, 200, {"THE"});
    surviveUi4.get<UiComponent>()->setZ(151);

    auto surviveUi5 = makeSentence(this, 485, 230, {"KING", 2.0f, {255, 215, 0, 255}});
    surviveUi5.get<UiComponent>()->setZ(151);

    auto surviveUi6 = makeSentence(this, 630, 65, {"TREASURY", 2.0f, {255, 215, 0, 255}});
    surviveUi6.get<UiComponent>()->setZ(151);

    auto surviveUi7 = makeSentence(this, 20, 430, {"LEFT CLICK TO PLACE A TOWER"});
    surviveUi7.get<UiComponent>()->setZ(151);

    auto surviveUi8 = makeSentence(this, 20, 460, {"RIGHT CLICK TO SEE THE HEIGHT OF A TOWER"});
    surviveUi8.get<UiComponent>()->setZ(151);

    auto surviveUi9 = makeSentence(this, 200, 495, {"EVERYTHING COST", 3.0f, {255, 0, 0, 255}, {0, 0, 0, 190}, {255, 255, 255, 165}});
    surviveUi9.get<UiComponent>()->setZ(151);
    auto surviveUi10 = makeSentence(this, 370, 590, {"2G", 3.0f, {255, 215, 0, 255}, {0, 0, 0, 190}, {255, 255, 255, 165}});
    surviveUi10.get<UiComponent>()->setZ(151);

    auto surviveUi11 = makeSentence(this, 660, 315, {"SHOP"});
    surviveUi11.get<UiComponent>()->setZ(151);
    auto surviveUi12 = makeSentence(this, 580, 345, {"AUTO REFRESH"});
    surviveUi12.get<UiComponent>()->setZ(151);

    auto surviveUi13 = makeSentence(this, 70, 230, {"CLICK", 2.0f, {255, 255, 255, 255}});
    surviveUi13.get<UiComponent>()->setZ(151);

    auto surviveUi14 = makeSentence(this, 105, 260, {"TO"});
    surviveUi14.get<UiComponent>()->setZ(151);

    auto surviveUi15 = makeSentence(this, 85, 290, {"PLAY", 3.0f, {255, 0, 0, 255}});
    surviveUi15.get<UiComponent>()->setZ(151);

    helperUi[0] = surviveUi0.get<UiComponent>();
    helperUi[1] = surviveUi.get<UiComponent>();
    helperUi[2] = surviveUi2.get<UiComponent>();
    helperUi[3] = surviveUi3.get<UiComponent>();
    helperUi[4] = surviveUi4.get<UiComponent>();
    helperUi[5] = surviveUi5.get<UiComponent>();
    helperUi[6] = surviveUi6.get<UiComponent>();
    helperUi[7] = surviveUi7.get<UiComponent>();
    helperUi[8] = surviveUi8.get<UiComponent>();
    helperUi[9] = surviveUi9.get<UiComponent>();
    helperUi[10] = surviveUi10.get<UiComponent>();
    helperUi[11] = surviveUi11.get<UiComponent>();
    helperUi[12] = surviveUi12.get<UiComponent>();
    helperUi[13] = surviveUi13.get<UiComponent>();
    helperUi[14] = surviveUi14.get<UiComponent>();
    helperUi[15] = surviveUi15.get<UiComponent>();

    // Log base gold
    gold = 2000.0f;

    auto goldGText = makeSentence(this, 740, 0, {"G", 4.0f, {255, 215, 0, 255}});
    goldGText.get<UiComponent>()->setZ(120);
    auto goldTextS = makeSentence(this, 600, 0, {std::to_string(static_cast<int>(gold)), 4.0f, {255, 255, 255, 255}});
    goldTextS.get<UiComponent>()->setZ(120);

    goldTextS.get<UiComponent>()->setRightAnchor(goldGText.get<UiComponent>()->left);

    goldText = goldTextS.get<SentenceText>();

    listenToEvent<TowerSelected>([this](const TowerSelected& event) {
        LOG_INFO(DOM, "Clicked TowerSelected: " << event.tower);
        selectedTower = event.tower;
    });

    listenToEvent<OnMouseClick>([this](const OnMouseClick& event) {
        if (paused)
            return;

        if (not startedUp)
            return;

        if (not executing)
            return;

        if (event.button != SDL_BUTTON_RIGHT)
            return;

        size_t x = posToTileMap(event.pos.x);
        size_t y = posToTileMap(event.pos.y);

        auto towerId = towerBoard[x][y].nbTowers;

        printHeight(x, y);

        if (towerId > 0)
        {
            LOG_INFO(DOM, "Tower stat: Damage = " << towerBoard[x][y].bulletPrototype.damage << ", Range: " <<
                towerBoard[x][y].range << ", Attack speed: " << towerBoard[x][y].shootInterval << ", Top tower health: " << towerBoard[x][y].towerData[towerId - 1].health);
        }

        // auto enemy = makeUiTexture(this, 40, 40, "player");

        // attach<CollisionComponent>(enemy.entity, 1, 1.0, 0);
        // attach<EnemyFlag>(enemy.entity, 5.0f, 1.0f);
        // attach<MoveToComponent>(enemy.entity, constant::Vector2D{792 / 2 + TILESIZE / 2.0f, 500 / 2 - TILESIZE / 2.0f}, 25.0f);

        // enemy.get<UiComponent>()->setX(event.pos.x);
        // enemy.get<UiComponent>()->setY(event.pos.y);
        // enemy.get<UiComponent>()->setZ(5);

        // auto enemy = makeUiTexture(this, 50, 50, "player");

        // enemy.get<UiComponent>()->setX(event.pos.x - 25);
        // enemy.get<UiComponent>()->setY(event.pos.y - 25);
        // enemy.get<UiComponent>()->setZ(1);
        // attach<CollisionComponent>(enemy.entity, 1, 0.5);
        // attach<EnemyFlag>(enemy.entity);
        // // attach<MoveToComponent>(enemy.entity, constant::Vector2D{300.0f, 300.0f}, 25.0f);

        // {
        //     std::lock_guard<std::mutex> lock(enemyCreationMutex);

        //     enemies.push_back(enemy.entity);
        //     // enemies.push_back(enemy.get<UiComponent>());
        // }
    });

    listenToEvent<NeedReroll>([this](const NeedReroll&) {
        // Todo roll 5 numbers and show them in the shop
        LOG_INFO(DOM, "Rerolling");

        LOG_INFO(DOM, towerDatas.size());

        for (size_t i = 0; i < 5; i++)
        {
            int selection = 1 + (randomNumber() * (towerDatas.size() - 1));

            shopSelection[i] = selection;

            const auto& data = towerDatas[selection];

            showItemInShop(i, data);
        }

        updatePreview();

        LOG_INFO(DOM, "Rerolled");
    });

    listenToEvent<TickEvent>([this](const TickEvent& event) {
        // LOG_INFO(DOM, "Tick received");

        elapsedSinceStart += event.tick;

        if (not startedUp)
            return;

        if (goldTextBackToWhiteFlag)
        {
            goldTextBackToWhite -= event.tick;

            if (goldTextBackToWhite < 0)
            {
                goldText->changeMainColor({255.0, 255.0, 255.0, 255.0});
                goldTextBackToWhiteFlag = false;
            }
        }

        if (paused)
            return;

        deltaTimeReceived += event.tick;

        counter += event.tick;

        if (counter >= 1000)
        {
            time += 1;
            minute = time / 60;
            sec = time % 60;
            std::string minute0 = minute < 10 ? "0" : "";
            std::string sec0 = sec < 10 ? "0" : "";

            if (minute >= 1 and not helpSpawn)
            {
                helpSpawn = true;

                auto helperTextSentence = makeSentence(this, 850, 450, {"THE HIGHER THE TOWER, THE HIGHER THE DAMAGE"});

                helperTextSentence.get<UiComponent>()->setZ(135);

                attach<MoveToComponent>(helperTextSentence.entity, constant::Vector2D{-450, 450}, 30.0f);
            }

            timerText->setText(minute0 + std::to_string(minute) + ":" + sec0 + std::to_string(sec));

            counter -= 1000;

            size_t nbEnemyToSpawn = minute * 1.5f + 1;
            // size_t nbEnemyToSpawn = 0;

            auto inc = time / 12;

            float enemyHealth = 3 * pow(1.1, inc);

            for (size_t i = 0; i < nbEnemyToSpawn; i++)
            {
                auto x = randomNumber() * 820 * 2; 
                auto y = randomNumber() * 400 * 2;

                x -= 410;
                y -= 200;

                if (x >= -30 and x < 410 and y >= -30 and y < 200)
                {
                    x = -30;
                    y = -30;
                }
                else if (x >= 410 and x < 850 and y >= -30 and y < 200)
                {
                    x = 850;
                    y = -30;
                }
                else if (x >= -30 and x < 410 and y >= 200 and y < 430)
                {
                    x = -30;
                    y = 430;
                }
                else if (x >= 410 and x < 850 and y >= 200 and y < 430)
                {
                    x = 850;
                    y = 430;
                }

                auto enemy = makeUiTexture(this, 40, 40, "player");

                attach<CollisionComponent>(enemy.entity, 1, 1.0, 0);
                attach<EnemyFlag>(enemy.entity, enemyHealth, 1.0f);
                attach<MoveToComponent>(enemy.entity, constant::Vector2D{792 / 2 + TILESIZE / 2.0f, 500 / 2 - TILESIZE / 2.0f}, 25.0f);

                enemy.get<UiComponent>()->setX(x);
                enemy.get<UiComponent>()->setY(y);
                enemy.get<UiComponent>()->setZ(enemyZ++);
                
                if (enemyZ >= 99)
                {
                    enemyZ = 2;
                }
            }
        }
    });

    listenToEvent<OnMouseMove>([this](const OnMouseMove& event) {
        // Todo add to all listenToEvent by default !
        if (not startedUp)
            return;

        if (paused)
            return;

        auto pos = event.pos;

        if (pos.y < 500)
        {
            size_t x = posToTileMap(pos.x);
            size_t y = posToTileMap(pos.y);

            if (currentSelectedTile.x != x or currentSelectedTile.y != y)
            {
                // std::string tileName = ((x + y) % 2) == 0 ? "tiles.selectedLight" : "tiles.selectedDark";
                // board[x][y].get<Texture2DComponent>()->setTexture(tileName);

                // std::string oldtileName = ((currentSelectedTile.x + currentSelectedTile.y) % 2) == 0 ? "tiles.light" : "tiles.dark";
                // board[currentSelectedTile.x][currentSelectedTile.y].get<Texture2DComponent>()->setTexture(oldtileName);

                currentSelectedTile.x = x;
                currentSelectedTile.y = y;

                const auto& data = towerDatas[shopSelection[selectedTower]];

                size_t highestTower = towerBoard[x][y].nbTowers;

                if (data.connectedTiles.size() != 0)
                {
                    for (const auto& pos : data.connectedTiles)
                    {
                        int tx = x + pos.x;
                        int ty = y + pos.y;

                        if (towerBoard[tx][ty].nbTowers > highestTower)
                            highestTower = towerBoard[tx][ty].nbTowers;
                    }
                };

                for (size_t j = 0; j < 5; j++)
                {
                    for (size_t i = 0; i < 5; i++)
                    {
                        preview[i][j].get<UiComponent>()->setX(tileMapToPos(x + i - 2));
                        preview[i][j].get<UiComponent>()->setY(tileMapToPos(y + j - 2) - highestTower * 3 * SCALE);
                        preview[i][j].get<UiComponent>()->setZ(3 + highestTower);
                    }
                }

                if (data.connectedTiles.size() != 0)
                {
                    for (const auto& pos : data.connectedTiles)
                    {
                        int tx = x + pos.x;
                        int ty = y + pos.y;

                        auto towerId = towerBoard[tx][ty].nbTowers;

                        // This is the prefix to get the smaller version of a tile !
                        int tHeight = towerId % 5;
                        std::string tHeightPrefix = "";

                        // Need to *2 as tilereda10 and 11 already exists !
                        tHeightPrefix = std::to_string(2 * (1 + tHeight));

                        preview[static_cast<int>(pos.x + 2)][static_cast<int>(pos.y + 2)].get<Texture2DComponent>()->setTexture(data.textureName + tHeightPrefix + std::to_string(static_cast<int>(pos.z)));
                    }
                }

            }
        }
    });

    listenToEvent<OnMouseClick>([this](const OnMouseClick& event) {
        if (end)
        {
            ecsRef->getSystem<SceneElementSystem>()->loadSystemScene<TitleScreen>();
        }

        if (not startedUp)
            return;

        if (not boardAnimationFinished)
            return;

        if (paused and elapsedSinceStart >= 810)
        {
            unpause();
            return;
        }
        else if (paused and elapsedSinceStart < 810)
        {
            unPausedRequested = true;
            return;
        }

        if (not executing)
            return;

        if (event.button != SDL_BUTTON_LEFT)
            return;

        if (event.pos.y >= 500)
            return;

        auto sTower = shopSelection[selectedTower];

        if (sTower < 0)
            return;

        size_t x = posToTileMap(event.pos.x);
        size_t y = posToTileMap(event.pos.y);

        // Tower is offset by the id(nb of tower in the cell) so that the tower are on each other
        // 2 is for the pixel size of the side
        const auto& towerData = towerDatas[sTower];

        if (towerData.baseCost > gold)
        {
            // Todo add sfx
            goldTextBackToWhite = 200;

            goldText->changeMainColor({255.0, 0, 0, 255.0});
            goldTextBackToWhiteFlag = true;

            auto sent = makeSentence(this, event.pos.x - 70, event.pos.y - 15, {"NOT ENOUGH GOLD", 1.0f, {255, 255, 255, 255}, {255, 0, 0, 255}});
            
            sent.get<UiComponent>()->setZ(125);

            attach<MoveToComponent>(sent.entity, constant::Vector2D{event.pos.x - 70, event.pos.y - 40}, 15.0f);

            ecsRef->sendEvent(PlaySoundEffect{"res/audio/blipSelect.wav"});

            return;
        }

        if (towerData.connectedTiles.size() > 0)
        {
            auto firstPos = towerData.connectedTiles[0];
            int firstX = x + firstPos.x;
            int firstY = y + firstPos.y;

            auto towerId = towerBoard[firstX][firstY].nbTowers;

            for (const auto& pos : towerData.connectedTiles)
            {
                int nextX = x + pos.x;
                int nextY = y + pos.y;

                // Todo set the 50 limit of the board as a define !
                if (nextX < 0 or nextX > 50 or nextY < 0 or nextY > 50)
                    return;

                auto nextTowerId = towerBoard[nextX][nextY].nbTowers;

                // Don't build if the height is not the same;
                // Todo add the posibility to have gap (air) as long as at least 2 points are touching the construct
                if (nextTowerId != towerId)
                {
                    auto sent = makeSentence(this, event.pos.x - 70, event.pos.y + 30, {"NOT SAME HEIGHT", 1.0f, {255, 255, 255, 255}, {255, 0, 0, 255}});
            
                    sent.get<UiComponent>()->setZ(125);

                    attach<MoveToComponent>(sent.entity, constant::Vector2D{event.pos.x - 70, event.pos.y + 40}, 5.0f);

                    for (const auto& pos : towerData.connectedTiles)
                    {
                        int nextX = x + pos.x;
                        int nextY = y + pos.y;

                        printHeight(nextX, nextY);
                    }

                    ecsRef->sendEvent(PlaySoundEffect{"res/audio/blipSelect.wav"});
                    
                    return;
                }

                if (nextTowerId > 0 and towerBoard[nextX][nextY].towerData[towerId - 1].canBeBuiltOn == false)
                {
                    auto sent = makeSentence(this, event.pos.x - 70, event.pos.y - 15, {"CANT BUILD HERE", 1.0f, {255, 255, 255, 255}, {255, 0, 0, 255}});
            
                    sent.get<UiComponent>()->setZ(125);

                    attach<MoveToComponent>(sent.entity, constant::Vector2D{event.pos.x - 70, event.pos.y - 40}, 15.0f);

                    ecsRef->sendEvent(PlaySoundEffect{"res/audio/blipSelect.wav"});

                    return;
                }
            }

            for (const auto& pos : towerData.connectedTiles)
            {
                int nextX = x + pos.x;
                int nextY = y + pos.y;

                placeTower(nextX, nextY, towerData.textureName + std::to_string(static_cast<int>(pos.z)), towerId, towerData);
            }

        }
        else
        {
            auto towerId = towerBoard[x][y].nbTowers;

            // Don't build on top of non buildable tower
            if (towerId > 0)
            {
                if (towerBoard[x][y].towerData[towerId - 1].canBeBuiltOn == false)
                {
                    auto sent = makeSentence(this, event.pos.x - 70, event.pos.y - 15, {"CANT BUILD HERE", 1.0f, {255, 255, 255, 255}, {255, 0, 0, 255}});
                
                    sent.get<UiComponent>()->setZ(125);

                    attach<MoveToComponent>(sent.entity, constant::Vector2D{event.pos.x - 70, event.pos.y - 40}, 15.0f);

                    ecsRef->sendEvent(PlaySoundEffect{"res/audio/blipSelect.wav"});

                    return;
                }
            }

            placeTower(x, y, towerData.textureName, towerId, towerData);
        }

        shopSelection[selectedTower] = 0;

        emptyItemInShop(selectedTower);

        shopSelected[selectedTower].get<UiComponent>()->setVisibility(false);

        selectedTower++;
        if (selectedTower >= 5)
        {
            selectedTower = 0;
            ecsRef->sendEvent(NeedReroll{});
        }
        else
        {
            updatePreview();
        }

        const auto& data = towerDatas[shopSelection[selectedTower]];

        size_t highestTower = towerBoard[x][y].nbTowers;

        if (data.connectedTiles.size() != 0)
        {
            for (const auto& pos : data.connectedTiles)
            {
                int tx = x + pos.x;
                int ty = y + pos.y;

                if (towerBoard[tx][ty].nbTowers > highestTower)
                    highestTower = towerBoard[tx][ty].nbTowers;
            }
        }

        for (size_t j = 0; j < 5; j++)
        {
            for (size_t i = 0; i < 5; i++)
            {
                preview[i][j].get<UiComponent>()->setY(tileMapToPos(y + j - 2) - highestTower * 3 * SCALE);
                preview[i][j].get<UiComponent>()->setZ(3 + highestTower);
            }
        }

        shopSelected[selectedTower].get<UiComponent>()->setVisibility(true);

        setGold(gold - towerData.baseCost);

        // attach<OnEventComponent>(towerBoard[x][y].entity);
        // towerBoard[x][y].get<UiComponent>()->setVisibility(true);
    });

    listenToEvent<CollisionEvent>([this](const CollisionEvent& event) {
        if (not startedUp)
            return;

        auto entity = ecsRef->getEntity(event.id1);
        auto entity2 = ecsRef->getEntity(event.id2);

        if (entity == nullptr or entity2 == nullptr)
            return;
        
        if (entity->has<EnemyFlag>() and entity2->has<EnemyFlag>())
            return;

        if (entity->has<PlayerFlag>() and entity2->has<EnemyFlag>())
        {
            ecsRef->removeEntity(entity2);

            int x = entity->get<PlayerFlag>()->x;
            int y = entity->get<PlayerFlag>()->y;

            towerBoard[x][y].popTower(); 

            if (towerBoard[x][y].nbTowers == 0)
            {
                cellWithTowers.erase(PagePos{x, y});

                // King pos, if no tower is left here then we lost !
                int kx = posToTileMap(820 / 2);
                int ky = posToTileMap((600 - 100) / 2);

                if (x == kx and y == ky)
                {
                    gameOver();
                }
            }
        }
        else if (entity2->has<PlayerFlag>() and entity->has<EnemyFlag>())
        {
            ecsRef->removeEntity(entity);

            int x = entity2->get<PlayerFlag>()->x;
            int y = entity2->get<PlayerFlag>()->y;

            towerBoard[x][y].popTower(); 

            if (towerBoard[x][y].nbTowers == 0)
            {
                cellWithTowers.erase(PagePos{x, y});

                int kx = posToTileMap(820 / 2);
                int ky = posToTileMap((600 - 100) / 2);

                if (x == kx and y == ky)
                {
                    ecsRef->sendEvent(PlaySoundEffect{"res/audio/explosion.wav"});

                    gameOver();
                }
            }
        }
        else if (entity->has<EnemyFlag>() and entity2->has<AllyBulletFlag>())
        {
            resolveCollisionEnemyAndBullet(entity, entity2);
        }
        else if (entity2->has<EnemyFlag>() and entity->has<AllyBulletFlag>())
        {
            resolveCollisionEnemyAndBullet(entity2, entity);
        }
        else if (entity->has<EnemyFlag>() and entity2->has<EnemyFlag>()) 
        {
            // if (not entity->has<UiComponent>() or not entity2->has<UiComponent>())
            //     return;

            // // Todo trash this

            // auto ui = entity->get<UiComponent>();
            // auto ui2 = entity2->get<UiComponent>();

            // float x, x1, w;
            // float y, y1, h;

            // float overlappingWidth, overlappingHeight;

            // if (ui->pos.x < ui2->pos.x)
            // {
            //     x = ui->pos.x;
            //     x1 = ui2->pos.x;
            //     w = ui->width;

            //     overlappingWidth = x + w - x1;
            // }
            // else
            // {
            //     x = ui2->pos.x;
            //     x1 = ui->pos.x;
            //     w = ui2->width;

            //     overlappingWidth = x + w - x1;
            // }

            // if (ui->pos.y < ui2->pos.y)
            // {
            //     y = ui->pos.y;
            //     y1 = ui2->pos.y;
            //     h = ui->height;

            //     overlappingHeight = y + h - y1;
            // }
            // else
            // {
            //     y = ui2->pos.y;
            //     y1 = ui->pos.y;
            //     h = ui2->height;

            //     overlappingHeight = y + h - y1;
            // }

            // if (overlappingWidth > overlappingHeight)
            // {
            //     if (ui->pos.x < ui2->pos.x)
            //         ui->setX(x - overlappingWidth);
            //     else
            //         ui2->setX(x - overlappingWidth);
            // }
            // else
            // {
            //     if (ui->pos.y < ui2->pos.y)
            //         ui->setY(y - overlappingHeight);
            //     else
            //         ui2->setY(y - overlappingHeight);
            // }

            // if (overlappingWidth > overlappingHeight)
            // {
            //     if (ui->pos.x < ui2->pos.x)
            //     {
            //         ui->setX(x - overlappingWidth / 2.0f + 5);
            //         ui2->setX(x + overlappingWidth / 2.0f + 5);
            //     }
            //     else
            //     {
            //         ui2->setX(x - overlappingWidth / 2.0f + 5);
            //         ui->setX(x + overlappingWidth / 2.0f + 5);
            //     }
            // }
            // else
            // {
            //     if (ui->pos.y < ui2->pos.y)
            //     {
            //         ui->setY(y - overlappingHeight / 2.0f + 5);
            //         ui2->setY(y + overlappingHeight / 2.0f + 5);
            //     }
            //     else
            //     {
            //         ui2->setY(y - overlappingHeight / 2.0f + 5);
            //         ui->setY(y + overlappingHeight / 2.0f + 5);
            //     }
            // }
        }
    });

    listenToEvent<TowerScalingDownEvent>([this](const TowerScalingDownEvent& event) {
        auto towerId = towerBoard[event.tile.x][event.tile.y].nbTowers;

        LOG_INFO(DOM, "Scaling down the tower at: " << event.tile.x << ", " << event.tile.y << " id : " << towerId);

        if (towerId < 6)
            return;

        for (size_t i = 6; i > 1; i--)
        {
            const auto& ent = towerBoard[event.tile.x][event.tile.y].towers[towerId - i];

            Texture2DComponent texture = *ent.get<Texture2DComponent>();

            Texture2DComponent textureFall = texture;
            textureFall.textureName = texture.textureName + "_fall0";

            Texture2DComponent textureFall2 = texture;
            textureFall2.textureName = texture.textureName + "_fall1";

            attach<Texture2DAnimationComponent>(ent, Texture2DAnimationComponent{{
                {100, textureFall, makeCallable<TowerScalingDownCallbackEvent>(event.tile, towerId - i, 1)},
                {200, textureFall2, makeCallable<TowerScalingDownCallbackEvent>(event.tile, towerId - i, 2, i == 2 ? false : true)},
            }, i == 6 ? true : false});
        }
    });

    listenToEvent<TowerScalingDownCallbackEvent>([this](const TowerScalingDownCallbackEvent& event) {
        auto& towerCell = towerBoard[event.tile.x][event.tile.y];

        for (size_t i = event.towerId + 1; i < towerCell.nbTowers; i++)
        {
            auto comp = towerCell.towers[i].get<UiComponent>();

            if (not comp.empty())
            {
                comp->setY(static_cast<float>(comp->pos.y) + event.descendingPixel * SCALE);
            }
        }

        if (event.startNext)
        {
            const auto& ent = towerBoard[event.tile.x][event.tile.y].towers[event.towerId + 1];

            auto comp = ent.get<Texture2DAnimationComponent>();

            if (not comp.empty())
            {
                comp->start();
            }
        }

    });

    listenToEvent<StartingTileMovedEvent>([this](const StartingTileMovedEvent& event) {
        waitingBoard[event.x][event.y]--;
        waitingStartingBoardAnimationFinish--;

        if (waitingBoard[event.x][event.y] == 0)
        {
            std::string tileName = ((event.x + event.y) % 2) == 0 ? "tiles.light" : "tiles.dark"; 

            board[event.x][event.y].get<Texture2DComponent>()->setTexture(tileName);
        }

        if (waitingStartingBoardAnimationFinish == 0)
        {
            boardAnimationFinished = true;
        }
    });

    // Player placement on the grid

    // placeTower(27, 23, "tiles.baseTower", 0, towerDatas[0]);
    // Todo set to player
    placeTower(posToTileMap(820 / 2), posToTileMap((640 - 100) / 2), "tiles.king", 0, towerDatas[0]);
}

void GameScene::printHeight(size_t mapX, size_t mapY)
{
    auto towerId = towerBoard[mapX][mapY].nbTowers;

    auto heightSent = makeSentence(this, 0, tileMapToPos(mapY), std::to_string(towerId));

    float x = tileMapToPos(mapX) + TILESIZE / 2.0f - heightSent.get<SentenceText>()->textWidth / 2.0f;
    float y = tileMapToPos(mapY) + TILESIZE / 2.0f - heightSent.get<SentenceText>()->textHeight / 2.0f;

    heightSent.get<UiComponent>()->setX(x);
    heightSent.get<UiComponent>()->setY(y);

    heightSent.get<UiComponent>()->setZ(110);

    attach<MoveToComponent>(heightSent.entity, constant::Vector2D{x, y - 10}, 5.0f);
}

void GameScene::placeTower(size_t x, size_t y, const std::string& textureName, size_t towerId, const TowerData& towerData)
{
    int towerOffset = towerId * 3 * SCALE;

    int yOffset = towerData.yOffset * SCALE;

    auto dustFx = makeUiTexture(this, 32 * SCALE, 16 * SCALE, "tiles.empty");
    dustFx.get<UiComponent>()->setX(tileMapToPos(x) + TILESIZE / 2.0f - 32 * SCALE / 2.0f);
    dustFx.get<UiComponent>()->setY(tileMapToPos(y) + TILESIZE / 2.0f - 16 * SCALE / 2.0f - towerOffset);
    dustFx.get<UiComponent>()->setZ(3);

    auto shadowFx = makeUiTexture(this, 16 * SCALE, 16 * SCALE, "tiles.empty");
    shadowFx.get<UiComponent>()->setX(tileMapToPos(x));
    shadowFx.get<UiComponent>()->setY(tileMapToPos(y) - towerOffset);
    shadowFx.get<UiComponent>()->setZ(3 + towerId);

    auto towerTex = makeUiTexture(this, TILESIZE, TILESIZE + towerData.yOffset * SCALE, "tiles.empty");

    towerTex.get<UiComponent>()->setX(tileMapToPos(x));
    towerTex.get<UiComponent>()->setY(tileMapToPos(y) - towerOffset - yOffset);
    towerTex.get<UiComponent>()->setZ(3);
    towerTex.get<UiComponent>()->setVisibility(false);

    towerBoard[x][y].addTower(towerTex.entity, towerData);

    if (towerData.connectedTiles.size() > 0)
    {
        colorBlock(towerTex, towerData);
    }

    if (towerBoard[x][y].nbTowers == 1)
        cellWithTowers.emplace(PagePos{static_cast<int>(x), static_cast<int>(y)}, &towerBoard[x][y]);

    ecsRef->sendEvent(PlaySoundEffect{"res/audio/random.wav"});

    std::function<void(const TickEvent&)> f = [this, dustFx, shadowFx, x, y, textureName, towerId, towerOffset, yOffset](const TickEvent& event) {
        if (not startedUp)
            return;

        vfxDuration[dustFx.entity.id].duration += event.tick;

        auto duration = vfxDuration[dustFx.entity.id].duration;

        auto& momentDone = vfxDuration[dustFx.entity.id].done;

        if (towerBoard[x][y].nbTowers <= towerId)
        {
            LOG_ERROR(DOM, "Tower already destroyed:"  << towerId);

            if (not momentDone[3])
            {
                auto ent = ecsRef->getEntity(shadowFx.entity.id);
                ecsRef->removeEntity(ent);
            }

            auto ent = ecsRef->getEntity(dustFx.entity.id);
            ecsRef->removeEntity(ent);

            return;
        }

        if (duration < 100 and not momentDone[0])
        {
            shadowFx.get<Texture2DComponent>()->setTexture("tiles.shadow0");

            towerBoard[x][y].towers[towerId].get<Texture2DComponent>()->setTexture(textureName);
            towerBoard[x][y].towers[towerId].get<UiComponent>()->setY(tileMapToPos(y) - 40 * SCALE - towerOffset - yOffset);
            towerBoard[x][y].towers[towerId].get<UiComponent>()->setZ(4 + towerId);
            towerBoard[x][y].towers[towerId].get<UiComponent>()->setVisibility(true);

            momentDone[0] = true;
        }
        else if (duration > 100 and duration <= 200 and not momentDone[1])
        {
            shadowFx.get<Texture2DComponent>()->setTexture("tiles.shadow1");
            towerBoard[x][y].towers[towerId].get<UiComponent>()->setY(tileMapToPos(y) - 30 * SCALE - towerOffset - yOffset);

            momentDone[1] = true;
        }
        else if (duration > 200 and duration <= 300 and not momentDone[2])
        {
            shadowFx.get<Texture2DComponent>()->setTexture("tiles.shadow2");
            towerBoard[x][y].towers[towerId].get<UiComponent>()->setY(tileMapToPos(y) - 15 * SCALE - towerOffset - yOffset);

            momentDone[2] = true;
        }
        else if (duration > 300 and not momentDone[3])
        {
            // shadowFx.get<Texture2DComponent>()->setTexture("tiles.simpleShadow");
            // shadowFx.get<UiComponent>()->setZ(2 + towerId);
            auto ent = ecsRef->getEntity(shadowFx.entity.id);
            ecsRef->removeEntity(ent);

            towerBoard[x][y].towers[towerId].get<UiComponent>()->setY(tileMapToPos(y) - towerOffset - yOffset);
            towerBoard[x][y].towers[towerId].get<UiComponent>()->setZ(3 + towerId);
            dustFx.get<Texture2DComponent>()->setTexture("tiles.dust0");

            if (towerId == 0)
            {
                auto shadow = makeUiTexture(this, TILESIZE, TILESIZE, "tiles.simpleShadow");

                shadow.get<UiComponent>()->setX(tileMapToPos(x));
                shadow.get<UiComponent>()->setY(tileMapToPos(y));
                shadow.get<UiComponent>()->setZ(2);

                attach<CollisionComponent>(shadow.entity, 0, 0.7f);
                attach<PlayerFlag>(shadow.entity, x, y);

                towerBoard[x][y].shadowEntity = shadow.entity;
            }

            momentDone[3] = true;
        }
        else if (duration > 400 and duration <= 500 and not momentDone[4])
        {
            dustFx.get<Texture2DComponent>()->setTexture("tiles.dust1");

            momentDone[4] = true;
        }
        else if (duration > 500 and duration <= 600 and not momentDone[5])
        {
            dustFx.get<Texture2DComponent>()->setTexture("tiles.dust2");

            momentDone[5] = true;
        }
        else if (duration > 600 and duration <= 700 and not momentDone[6])
        {
            dustFx.get<Texture2DComponent>()->setTexture("tiles.dust3");

            momentDone[6] = true;
        }
        else if (duration > 700 and not momentDone[7])
        {
            if (towerId > 0 and towerId % 5 == 0)
                ecsRef->sendEvent(TowerScalingDownEvent{x, y});

            // dustFx.get<UiComponent>()->setVisibility(false);
            auto ent = ecsRef->getEntity(dustFx.entity.id);
            ecsRef->removeEntity(ent);

            momentDone[7] = true;                
        }
    };

    attach<OnEventComponent>(dustFx.entity, f);
}

void GameScene::updatePreview()
{
    for (size_t y = 0; y < 5; y++)
    {
        for (size_t x = 0; x < 5; x++)
        {
            preview[x][y].get<Texture2DComponent>()->setTexture("tiles.empty");
        }
    }

    size_t baseX = 2;
    size_t baseY = 2;

    const auto& data = towerDatas[shopSelection[selectedTower]];

    if (data.connectedTiles.size() == 0)
    {
        preview[baseX][baseY].get<Texture2DComponent>()->setTexture(data.textureName);
    }
    else
    {
        for (const auto& pos : data.connectedTiles)
        {
            int nextX = baseX + pos.x;
            int nextY = baseY + pos.y;

            if (nextX < 0 or nextX >= 5 or nextY < 0 or nextY >= 5)
            {
                LOG_ERROR(DOM, "Pos out of bound, Tower is not valid: " << data.textureName);
                return;
            }

            int towerId = towerBoard[static_cast<int>(currentSelectedTile.x + pos.x)][static_cast<int>(currentSelectedTile.y + pos.y)].nbTowers;

            // This is the prefix to get the smaller version of a tile !
            int tHeight = towerId % 5;
            std::string tHeightPrefix = "";

            // Need to *2 as tilereda10 and 11 already exists !
            tHeightPrefix = std::to_string(2 * (1 + tHeight));

            preview[nextX][nextY].get<Texture2DComponent>()->setTexture(data.textureName + tHeightPrefix + std::to_string(static_cast<int>(pos.z)));

            colorBlock(preview[nextX][nextY], data);
        }
    }
}

void GameScene::execute()
{
    if (waitingStartingBoardAnimationFinish == 0 and boardAnimationFinished and paused)
    {
        auto tx = posToTileMap(820 / 2);
        auto ty = posToTileMap((640 - 160) / 2);

        placeTower(tx, ty, "tiles.king", 0, towerDatas[0]);

        paused = false;
    }

    if (elapsedSinceStart >= 810 and unPausedRequested and boardAnimationFinished and paused)
        unpause();

    if (paused)
        return;

    if (deltaTimeReceived == 0)
        return;

    executing = true;

    for (auto tower : cellWithTowers)
    {
        // LOG_INFO(DOM, "Shooting");

        // auto towerId = tower.second->nbTowers - 1;

        auto& towerCell = tower.second;

        if (towerCell->bulletPrototype.damage == 0.0f)
            continue;

        towerCell->currentCd += deltaTimeReceived;

        // LOG_INFO(DOM, "Tower current cd:" << towers[i].currentCd << ", shooting interval: " << towers[i].shootInterval);

        // Todo set range in tower (range is in pixel)
        float range = towerCell->range;

        while (towerCell->currentCd >= towerCell->shootInterval)
        {
            towerCell->currentCd -= towerCell->shootInterval;

            bool enemySpotted = false;

            float distance = 0.0f;
            constant::Vector2D endPos;

            float tx = tileMapToPos(towerCell->x);
            float ty = tileMapToPos(towerCell->y);

            for (const auto& comp : ecsRef->view<EnemyFlag>())
            {
                if (comp->entity->has<UiComponent>())
                {
                    float x = comp->entity->get<UiComponent>()->pos.x + comp->entity->get<UiComponent>()->width / 2.0f;
                    float y = comp->entity->get<UiComponent>()->pos.y + comp->entity->get<UiComponent>()->height / 2.0f;

                    float diffX = tx - x;
                    float diffY = ty - y;

                    float currentDist = diffX * diffX + diffY * diffY;

                    // LOG_INFO(DOM, "x: " << x << ", y: " << y << ", tower x:" << towerInfo.x * TILESIZE - BOARDOFFSET << ", tower y: " << towerInfo.y * TILESIZE - BOARDOFFSET << ", dist: " << currentDist);

                    if (not enemySpotted)
                    {
                        distance = currentDist;
                        endPos.x = diffX;
                        endPos.y = diffY;

                        enemySpotted = true;
                    }
                    else
                    {
                        if (distance > currentDist)
                        {
                            distance = currentDist;
                            endPos.x = diffX;
                            endPos.y = diffY;
                        }
                    }
                }

            }

            distance = std::sqrt(distance);

            if (enemySpotted and distance < range)
            {
                endPos.x = tx - endPos.x / distance * range;
                endPos.y = ty - endPos.y / distance * range;

                auto bullet = makeUiTexture(this, 12, 12, "baseBullet");

                bullet.get<UiComponent>()->setX(tx + TILESIZE / 2.0f - 11);
                bullet.get<UiComponent>()->setY(ty + TILESIZE / 2.0f - 6);
                bullet.get<UiComponent>()->setZ(2);

                ecsRef->sendEvent(PlaySoundEffect{"res/audio/laserShoot.wav"});

                attach<AllyBulletFlag>(bullet.entity, towerCell->bulletPrototype.damage);
                attach<CollisionComponent>(bullet.entity, 2, 1.5, 1);

                attach<MoveToComponent>(bullet.entity, endPos, 500.0f);
            }            
        }
    }

    deltaTimeReceived = 0;
}

void GameScene::resolveCollisionEnemyAndBullet(Entity* enemy, Entity* bullet)
{
    LOG_INFO(DOM, "Bullet Hit");

    auto enemyFlag = enemy->get<EnemyFlag>();
    auto bulletFlag = bullet->get<AllyBulletFlag>();

    enemyFlag->health -= bulletFlag->damage;

    if (enemyFlag->health < 0.0f)
    {
        setGold(gold + enemyFlag->baseGoldDrop);
        ecsRef->removeEntity(enemy);
    }

    ecsRef->removeEntity(bullet);
}

void GameScene::emptyItemInShop(size_t shopCase)
{
    for (size_t i = 0; i < 5; i++)
    {
        for (size_t j = 0; j < 5; j++)
        {
            shopItemHolder[shopCase][i][j]->setTexture("tiles.empty");
        }
    }
}

void GameScene::showItemInShop(size_t shopCase, const TowerData& data)
{
    emptyItemInShop(shopCase);

    size_t baseX = 2;
    size_t baseY = 2;

    if (data.connectedTiles.size() == 0)
    {
        shopItemHolder[shopCase][baseX][baseY]->setTexture(data.textureName);
    }
    else
    {
        for (const auto& pos : data.connectedTiles)
        {
            int nextX = baseX + pos.x;
            int nextY = baseY + pos.y;

            if (nextX < 0 or nextX >= 5 or nextY < 0 or nextY >= 5)
            {
                LOG_ERROR(DOM, "Pos out of bound, Tower is not valid: " << data.textureName);
                return;
            } 

            shopItemHolder[shopCase][nextX][nextY]->setTexture(data.textureName + std::to_string(static_cast<int>(pos.z)));

            colorBlock(shopItemHolder[shopCase][nextX][nextY], data);
        }
    }
}

void GameScene::unpause()
{
    int x = posToTileMap(792 / 2);
    int y = posToTileMap((600 - 100) / 2);
    towerBoard[x][y].popTower();

    SCALE = TRUESCALE;
    TILESIZE = REALTILESIZE * SCALE;

    popped = true;

    size_t visibleXMin = posToTileMap(0);
    size_t visibleYMin = posToTileMap(0);

    size_t visibleXMax = posToTileMap(820);
    size_t visibleYMax = posToTileMap(640);

    waitingStartingBoardAnimationFinish = 0;

    for (size_t i = 0; i < 50; i++)
    {
        for (size_t j = 0; j < 50; j ++)
        {
            size_t posY = tileMapToPos(j);

            auto rng = static_cast<int>(randomNumber() * 9);

            if (i >= visibleXMin and i <= visibleXMax and j >= visibleYMin and j <= visibleYMax)
            {
                posY = 540 + rng * 48;
            }

            board[i][j].get<UiComponent>()->setWidth(TILESIZE);
            board[i][j].get<UiComponent>()->setHeight(TILESIZE);

            board[i][j].get<UiComponent>()->setX(tileMapToPos(i));
            board[i][j].get<UiComponent>()->setY(posY);

            if (i >= visibleXMin and i <= visibleXMax and j >= visibleYMin and j <= visibleYMax)
            {
                auto callback = makeCallable<StartingTileMovedEvent>(i, j);

                attach<MoveToComponent>(board[i][j], constant::Vector2D{tileMapToPos(i), tileMapToPos(j)}, 500.0f, 0.0f, false, callback);

                std::vector<Animation2DKeyPoint> keypoints;

                std::string tileName = ((i + j) % 2) == 0 ? "tiles.light" : "tiles.dark"; 

                for (int i = 0; i < 5; i++)
                {
                    keypoints.emplace_back(i * 150, tileName + std::to_string(i), i == 4 ? callback : nullptr);
                }

                attach<Texture2DAnimationComponent>(board[i][j], Texture2DAnimationComponent{keypoints, true});

                waitingBoard[i][j] = 2;

                waitingStartingBoardAnimationFinish += 2;

                boardAnimationFinished = false;
            }
        }
    }

    obsc->setVisibility(false);
    
    for (size_t i = 0; i < 16; i++)
    {
        helperUi[i]->setVisibility(false);
    }
}

void GameScene::generateTowerData()
{
    for (const auto& layout : towerLayout)
    {
        for (size_t i = 0; i < 4; i++)
        {
            TowerData data;

            data.textureName = "tiles.towerReda";

            TowerColor color = static_cast<TowerColor>(i);

            data.color = color;

            data.health = 3;

            if (color == TowerColor::Red)
            {
                data.damageMultiplier = 1.1f;
            }
            else if (color == TowerColor::Blue)
            {
                data.rangeMultiplier = 1.1f;
            }
            else if (color == TowerColor::Green)
            {
                data.health = 5;
                data.healthMultiplier = 1.2f;
            }
            else if (color == TowerColor::Purple)
            {
                data.shootIntervalMultiplier = 0.9f;
            }

            data.connectedTiles = layout.connectedTiles;

            towerDatas.push_back(data);
        }
    }
}

void GameScene::gameOver()
{
    LOG_INFO(DOM, "GAME OVER");
    paused = true;
    startedUp = false;
    end = true;

    obsc->setVisibility(true);

    if (minute >= 10)
    {
        auto W = makeSentence(this, 200, 100, {"YOU WIN", 6.0f, {0.0f, 255.0f, 0.0f, 255.0}});

        W.get<UiComponent>()->setZ(170);
    }
    else
    {
        auto L = makeSentence(this, 185, 100, {"YOU LOST", 6.0f, {255.0f, 0.0f, 0.0f, 255.0}});

        L.get<UiComponent>()->setZ(170);
    }

    auto mainMenu = makeSentence(this, 80, 400, {"CLICK TO GO TO TITLESCREEN"});

    mainMenu.get<UiComponent>()->setZ(170);
}