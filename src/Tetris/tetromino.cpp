#include "tetromino.h"

#include "2D/texture.h"

#include "logger.h"

#include <random>
#include <algorithm>

using namespace pg;

namespace
{
    static constexpr const char * const DOM = "Tetromino";

    std::string tetrominoTypeToString(const TetrominoType& type)
    {
        switch (type)
        {
            case TetrominoType::I:
            {
                return "I";
                break;
            }
            
            case TetrominoType::O:
            {
                return "O";
                break;
            }
                
            case TetrominoType::J:
            {
                return "J";
                break;
            }

            case TetrominoType::L:
            {
                return "L";
                break;
            }

            case TetrominoType::S:
            {
                return "S";
                break;
            }

            case TetrominoType::Z:
            {
                return "Z";
                break;
            }

            case TetrominoType::T:
            {
                return "T";
                break;
            }
        }

        return "Ghost";
    }
}

Tetromino::Tetromino(const TetrominoType& type) : type(type)
{
    
}

void Tetromino::setMino(const TetrominoType& type)
{
    this->type = type;

    switch (type)
    {
        case TetrominoType::I:
        {
            nbPossibleRotation = 2;

            uint8_t rota0[4][4] = {{0, 0, 1, 0},
                                   {0, 0, 1, 0},
                                   {0, 0, 1, 0},
                                   {0, 0, 1, 0}};

            possibleRotation[0] = rota0;

            uint8_t rota1[4][4] = {{0, 0, 0, 0},
                                   {0, 0, 0, 0},
                                   {0, 0, 0, 0},
                                   {1, 1, 1, 1}};

            possibleRotation[1] = rota1;
            posX = 4;
            posY = 20;
            break;
        }
        
        case TetrominoType::O:
        {
            nbPossibleRotation = 1;

            uint8_t rota0[4][4] = {{0, 0, 0, 0},
                                   {0, 0, 0, 0},
                                   {0, 1, 1, 0},
                                   {0, 1, 1, 0}};

            possibleRotation[0] = rota0;
            posX = 4;
            posY = 20;
            break;
        }
            
        case TetrominoType::J:
        {
            nbPossibleRotation = 4;

            uint8_t rota0[4][4] = {{0, 0, 0, 0},
                                   {0, 0, 0, 0},
                                   {0, 1, 0, 0},
                                   {0, 1, 1, 1}};

            possibleRotation[0] = rota0;

            uint8_t rota1[4][4] = {{0, 0, 0, 0},
                                   {0, 0, 1, 0},
                                   {0, 0, 1, 0},
                                   {0, 1, 1, 0}};

            possibleRotation[1] = rota1;

            uint8_t rota2[4][4] = {{0, 0, 0, 0},
                                   {0, 0, 0, 0},
                                   {1, 1, 1, 0},
                                   {0, 0, 1, 0}};

            possibleRotation[2] = rota2;

            uint8_t rota3[4][4] = {{0, 0, 0, 0},
                                   {0, 1, 1, 0},
                                   {0, 1, 0, 0},
                                   {0, 1, 0, 0}};

            possibleRotation[3] = rota3;
            posX = 4;
            posY = 20;
            break;
        }

        case TetrominoType::L:

            break;

        case TetrominoType::S:

            break;

        case TetrominoType::Z:

            break;

        case TetrominoType::T:

            break;
    }
}

void Tetromino::createTexture(EntitySystem *ecsRef)
{
    for (size_t i = 0; i < 4; i++)
    {
        // ref[i] = ecsRef->createEntity();

        auto typeStr = tetrominoTypeToString(type);

        LOG_INFO(DOM, "Creating texture [" << i << "] of type: " << typeStr);

        makeUiTexture(ecsRef, 24, 24, typeStr);
    }
}

RandomTetrominoGenerator::RandomTetrominoGenerator()
{
    bag1 = {TetrominoType::I, TetrominoType::O, TetrominoType::J, TetrominoType::L, TetrominoType::S, TetrominoType::Z, TetrominoType::T};
    bag2 = {TetrominoType::I, TetrominoType::O, TetrominoType::J, TetrominoType::L, TetrominoType::S, TetrominoType::Z, TetrominoType::T};

    std::random_device rd;
    std::mt19937 g(rd());

    index = std::rand() % 14;

    std::shuffle(bag1.begin(), bag1.end(), g);
    std::shuffle(bag2.begin(), bag2.end(), g);
}

TetrominoType RandomTetrominoGenerator::generateTetromino()
{
    TetrominoType type;

    if (index < 7)
        type = bag1[index];
    else
        type = bag2[index - 7];

    index++;

    if (index == 7)
    {
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(bag1.begin(), bag1.end(), g);
    }
    else if (index == 14)
    {
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(bag2.begin(), bag2.end(), g);

        index = 0;
    }
 
    return type;
}

TetrominoType RandomTetrominoGenerator::preview(size_t next)
{
    auto nextIndex = (index + next) % 14;

    if (nextIndex < 7)
        return bag1[nextIndex];
    else 
        return bag2[nextIndex];
}

void GameCanvas::init()
{
    auto entity = ecsRef->createEntity();

    timer = ecsRef->attach<Timer>(entity);

    timer->interval = 1000;

    timer->callback = makeCallable<FallTimeout>();

    timer->running = true;

    for (size_t i = 0; i < 10; i++)
    {
        for (size_t j = 0; j < 24; j++)
        {
            auto tex = makeUiTexture(ecsRef, 24, 24, "Ghost");

            auto ui = tex.get<UiComponent>();

            ui->setX(24 * i);
            ui->setY(480 - 24 * j);

            entityCanvas[i][j] = tex.entity;

            canvas[i][j] = -1;
        }
    }
}

bool GameCanvas::moveHelper(int x, int y)
{
    bool move = true;

    for (uint8_t i = 0; i < 4; i++)
    {
        for (uint8_t j = 0; j < 4; j++)
        {
            if (currentMino.possibleRotation[currentMino.rotation].pos[i][j] == 1)
            {
                if (canvas[currentMino.posX + x + i][currentMino.posY + y - j] != -1)
                {
                    move = false;
                }
            }
        }
    }

    return move;
}

void GameCanvas::moveDown()
{
    bool move = true;

    if (currentMino.posY < 4)
    {
        move = false;
    }
    else
    {
        move = moveHelper(0, -1);
    }

    if (move)
    {
        setMino(-1, "Ghost");

        currentMino.posY--;

        setMino(static_cast<uint8_t>(currentMino.type), tetrominoTypeToString(currentMino.type));
    }
    else
    {
        startLockTimer();
    }
}

void GameCanvas::moveLeft()
{
    bool move = true;

    if (currentMino.posX <= 0)
    {
        move = false;
    }
    else
    {
        move = moveHelper(-1, 0);
    }

    if (move)
    {
        setMino(-1, "Ghost");

        currentMino.posX--;

        setMino(static_cast<uint8_t>(currentMino.type), tetrominoTypeToString(currentMino.type));
    }
    else
    {
        startLockTimer();
    }
}

void GameCanvas::moveUp()
{
    bool move = true;

    if (currentMino.posY >= 20)
    {
        move = false;
    }
    else
    {
        move = moveHelper(0, 1);
    }

    if (move)
    {
        setMino(-1, "Ghost");

        currentMino.posY++;

        setMino(static_cast<uint8_t>(currentMino.type), tetrominoTypeToString(currentMino.type));
    }
    else
    {
        startLockTimer();
    }
}

void GameCanvas::moveRight()
{
    bool move = true;

    if (currentMino.posY > 5)
    {
        move = false;
    }
    else
    {
        move = moveHelper(1, 0);
    }

    if (move)
    {
        setMino(-1, "Ghost");

        currentMino.posX++;

        setMino(static_cast<uint8_t>(currentMino.type), tetrominoTypeToString(currentMino.type));
    }
    else
    {
        startLockTimer();
    }
}

void GameCanvas::rotate()
{
    setMino(-1, "Ghost");

    bool placeable = false;

    currentMino.rotation = (currentMino.rotation + 1) % currentMino.nbPossibleRotation;

    placeable = moveHelper(0, 0);
    
    while (not placeable)
    {
        placeable = moveHelper(0, 1);
    }

    setMino(static_cast<uint8_t>(currentMino.type), tetrominoTypeToString(currentMino.type));

    // resetLockTimer();
}

void GameCanvas::setMino(int value, const std::string& texture)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        for (uint8_t j = 0; j < 4; j++)
        {
            if (currentMino.possibleRotation[currentMino.rotation].pos[i][j] == 1)
            {
                canvas[currentMino.posX + i][currentMino.posY - j] = value;
                entityCanvas[currentMino.posX + i][currentMino.posY - j]->get<Texture2DComponent>()->setTexture(texture);
            }
        }
    }
}

void GameCanvas::onEvent(const OnSDLGamepadPressed& event)
{
    if (event.button == SDL_CONTROLLER_BUTTON_A)
    {
        LOG_INFO(DOM, "Game canvas A Pressed");

        spawnTetromino();
    }
}

void GameCanvas::onEvent(const FallTimeout& event)
{
    LOG_INFO(DOM, "Fall timeout: ");
}

void GameCanvas::spawnTetromino()
{
    LOG_INFO(DOM, "Generate Tetromino");

    auto type = generator.generateTetromino();

    auto ent = entityCanvas[5][5];

    // ecsRef->sendEvent(TextureChangeEvent{ent->id, "Ghost", "I"});

    LOG_INFO(DOM, "Got entity: " << ent->id << " but tex comp has id: " << ent->get<Texture2DComponent>().entityId);

    LOG_INFO(DOM, "Entity in tex: " << ent->get<Texture2DComponent>()->entity.id);

    if (not ent->get<Texture2DComponent>()->entity.empty())
    {
        LOG_INFO(DOM, "Tex entity is correctly loaded");
    }

    ent->get<Texture2DComponent>()->setTexture("I");

    LOG_INFO(DOM, "New Tetromino: " << static_cast<uint8_t>(type));

    currentMino.setMino(type);

    LOG_INFO(DOM, "New Tetromino: " << static_cast<uint8_t>(currentMino.type));

    for (auto i = 0; i < 4; i++)
    {
        for (auto j = 0; j < 4; j++)
        {
            if (currentMino.possibleRotation[currentMino.rotation].pos[i][j] != 0)
            {
                canvas[currentMino.posX + i][currentMino.posY - j] = static_cast<uint8_t>(currentMino.type);
                auto ent = entityCanvas[i][j];

                ent->get<Texture2DComponent>()->setTexture(tetrominoTypeToString(currentMino.type));
            }
        }
    }

    // currentMino.createTexture(world());
}