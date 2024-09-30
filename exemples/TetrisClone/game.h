#pragma once

#include "Scene/scenemanager.h"
#include "2D/animator2d.h"
#include "2D/collisionsystem.h"
#include "2D/texture.h"
#include "UI/sentencesystem.h"

#include "Systems/basicsystems.h"

#include <mutex>

using namespace pg;

struct NeedReroll {};

// Todo enable flag registration
struct PlayerFlag
{
    PlayerFlag(size_t x, size_t y) : x(x), y(y) {}
    PlayerFlag(const PlayerFlag& other) : x(other.x), y(other.y) {}

    size_t x;
    size_t y;
};
struct EnemyFlag : public Ctor
{
    EnemyFlag(float health = 1.0f, float baseGoldDrop = 1.0f) : health(health), baseGoldDrop(baseGoldDrop) { }
    EnemyFlag(const EnemyFlag& other) : health(other.health), baseGoldDrop(other.baseGoldDrop), entity(other.entity) { }

    virtual void onCreation(EntityRef entity) override
    {
        this->entity = entity;
    }

    float health = 1.0f;
    float baseGoldDrop = 1.0f;

    EntityRef entity;
};

struct AllyBulletFlag
{
    AllyBulletFlag(float damage = 1.0f) : damage(damage) {}
    AllyBulletFlag(const AllyBulletFlag& other) : damage(other.damage) { }

    float damage = 1.0f;
};

struct PlayerCollision {};

struct Bullet
{
    float damage = 0.0f;
};

enum class TowerColor : uint8_t
{
    Red    = 0, // Fire, attack focus
    Green  = 1, // Earth, Health focus
    Blue   = 2, // Water, Range focus
    Purple = 3, // Air, attack speed focus
    Gold,
    Gray,
    Black,
    White,
};

struct TowerLayout
{
    std::vector<constant::Vector3D> connectedTiles = {};
};

struct TowerData
{
    TowerData() {};
    TowerData(const std::string& textureName, float health) : textureName(textureName), color(TowerColor::Gold), health(health) {}
    TowerData(const TowerData& other) : textureName(other.textureName), color(other.color), health(other.health), damage(other.damage), damageMultiplier(other.damageMultiplier), healthMultiplier(other.healthMultiplier),
        rangeMultiplier(other.rangeMultiplier), shootIntervalMultiplier(other.shootIntervalMultiplier), canBeBuiltOn(other.canBeBuiltOn), yOffset(other.yOffset),
        connectedTiles(other.connectedTiles), baseCost(other.baseCost) {}

    std::string textureName;

    TowerColor color = TowerColor::Red;

    // Health of the current piece (don't carry to the next tower)
    float health = 0;

    // Added flat damage (carries to the next towers)
    float damage = 1;
    
    // Multipliers (carries to the next towers)
    float damageMultiplier = 1.0;
    float healthMultiplier = 1.0f;
    float rangeMultiplier = 1.0f;
    float shootIntervalMultiplier = 1.0f;

    bool canBeBuiltOn = true;

    float yOffset = 0;

    // x and y are coordinates relative to the main tile and z is the index of the texture
    std::vector<constant::Vector3D> connectedTiles = {};

    float baseCost = 2.0f;
};

struct SelectedTile
{
    size_t x = 0;
    size_t y = 0;
};

struct TowerScalingDownEvent
{
    SelectedTile tile;
};

struct TowerScalingDownCallbackEvent
{
    TowerScalingDownCallbackEvent(const SelectedTile& tile, size_t id, size_t descendingPixel, bool startNext = false) : tile(tile), towerId(id), descendingPixel(descendingPixel), startNext(startNext) {}
    TowerScalingDownCallbackEvent(const TowerScalingDownCallbackEvent& other) : tile(other.tile), towerId(other.towerId), descendingPixel(other.descendingPixel), startNext(other.startNext) {}

    SelectedTile tile;

    size_t towerId;

    size_t descendingPixel;

    bool startNext;
};

struct TowerCell
{
    // Pos
    size_t x = 0, y = 0;

    // Those are base value
    size_t range = 200;
    size_t shootInterval = 1000;
    size_t currentCd = 0;

    float generalHealthMultiplier = 1.0;

    size_t nbTowers = 0;
    
    std::vector<EntityRef> towers;
    std::vector<TowerData> towerData;

    void addTower(const EntityRef& entity, const TowerData& data)
    {
        // LOG_INFO("Tower cell", "Added tower at: " << x << ", y: " << y);
        nbTowers++;
        towers.push_back(entity);

        registerTower(data);
    }

    void popTower()
    {
        // LOG_INFO("Tower cell", "Trying tower: " << x << ", y: " << y << ", nbTowers: " << nbTowers);

        if (nbTowers == 0)
            return;

        // LOG_INFO("Tower cell", "Popping tower");

        towerData.pop_back();
        auto tower = towers.back();
        towers.pop_back();

        tower->world()->removeEntity(tower);

        nbTowers--;

        if (nbTowers == 0)
        {
            shadowEntity->world()->removeEntity(shadowEntity);
        }
    }

    void registerTower(TowerData data);

    EntityRef shadowEntity;

    Bullet bulletPrototype;
};

struct VFX
{
    size_t duration = 0;
    bool done[8] = { false };
};

struct GameScene : public Scene
{
    virtual void init() override;
    // virtual void startUp() override;

    virtual void startUp() override { startedUp = true; ecsRef->sendEvent(NeedReroll{}); }

    bool startedUp = false;

    virtual void execute() override;

    bool executing = false;

    void resolveCollisionEnemyAndBullet(Entity* enemy, Entity* bullet);

    void placeTower(size_t x, size_t y, const std::string& textureName, size_t towerId, const TowerData& towerData);

    void updatePreview();

    void printHeight(size_t mapX, size_t mapY);

    size_t counter;

    int deltaTimeReceived = 0;

    std::mutex enemyCreationMutex;

    EntityRef board[50][50];
    // EntityRef towerBoard[50][50];
    TowerCell towerBoard[50][50];

    // std::vector<TowerCell> towers;

    std::map<PagePos, TowerCell*> cellWithTowers;

    SelectedTile currentSelectedTile;

    int selectedTower = 0;

    size_t minute = 0;
    size_t sec = 0;

    std::map<_unique_id, VFX> vfxDuration;

    void setGold(float gold) { this->gold = gold; goldText->setText(std::to_string(static_cast<int>(gold))); }

    CompRef<SentenceText> goldText;
    float gold = 0.0f;

    bool goldTextBackToWhiteFlag = false;
    int goldTextBackToWhite = 0;

    void emptyItemInShop(size_t shopCase);

    void showItemInShop(size_t shopCase, const TowerData& data);

    bool raiseShopFlag = false;
    bool lowerShopFlag = false;

    EntityRef shopSelected[5];
    int shopSelection[5] = {0};

    CompRef<Texture2DComponent> shopItemHolder[5][5][5];

    uint64_t time;
    CompRef<SentenceText> timerText;

    size_t enemyZ = 2;

    EntityRef preview[5][5];

    CompRef<UiComponent> obsc;
    CompRef<UiComponent> helperUi[16];

    bool paused = true;

    void unpause();

    void generateTowerData();

    std::vector<TowerData> towerDatas = { TowerData{"tiles.king", 100.0f} };

    bool unPausedRequested = false;
    bool ruleRead = false;
    size_t elapsedSinceStart = 0;

    void gameOver();

    bool end = false;

    bool placeKing = false;
    bool popped = false;

    bool helpSpawn = false;

    size_t waitingBoard[50][50] = {0};
    size_t waitingStartingBoardAnimationFinish = 1;
    bool boardAnimationFinished = true;
};