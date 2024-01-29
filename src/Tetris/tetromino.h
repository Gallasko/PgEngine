#pragma once

#include <string>
#include <vector>

#include "ECS/system.h"
#include "Input/inputcomponent.h"

#include "Systems/coresystems.h"

using namespace pg;

enum class TetrominoType : uint8_t
{
    I = 0,
    O,
    J,
    L,
    S,
    Z,
    T
};

struct TetrominoPos
{
    TetrominoPos(uint8_t v[4][4])
    {
        pos[0][0] = v[0][0]; pos[0][1] = v[0][1]; pos[0][2] = v[0][2]; pos[0][3] = v[0][3];
        pos[1][0] = v[1][0]; pos[1][1] = v[1][1]; pos[1][2] = v[1][2]; pos[1][3] = v[1][3];
        pos[2][0] = v[2][0]; pos[2][1] = v[2][1]; pos[2][2] = v[2][2]; pos[2][3] = v[2][3];
        pos[3][0] = v[3][0]; pos[3][1] = v[3][1]; pos[3][2] = v[3][2]; pos[3][3] = v[3][3];
    }

    TetrominoPos() {}

    uint8_t pos[4][4];
};

struct Tetromino
{
    Tetromino(const TetrominoType& type);

    void setMino(const TetrominoType& type);

    void createTexture(EntitySystem *ecsRef);

    TetrominoType type;

    uint8_t rotation = 0;

    uint8_t nbPossibleRotation;
    TetrominoPos possibleRotation[4];

    EntityRef ref[4];

    uint8_t posX, posY;
};

struct RandomTetrominoGenerator
{
    RandomTetrominoGenerator();

    TetrominoType generateTetromino();

    TetrominoType preview(size_t next);

    size_t index = 0;

    std::vector<TetrominoType> bag1;
    std::vector<TetrominoType> bag2;
};

struct FallTimeout {};

struct GameCanvas : public System<Listener<OnSDLGamepadPressed>, Listener<FallTimeout>, InitSys>
{
    GameCanvas() : currentMino(TetrominoType::I) {}

    virtual void init() override;

    virtual void onEvent(const OnSDLGamepadPressed& event) override;

    virtual void onEvent(const FallTimeout& event) override;

    void spawnTetromino();

    CompRef<Timer> timer;

    RandomTetrominoGenerator generator;

    Tetromino currentMino;

    // const size_t width = 10, height = 20;

    // uint8_t canvas[width][height];
    // _unique_id entityCanvas[width][height];

    int canvas[10][24];
    EntityRef entityCanvas[10][24];
};