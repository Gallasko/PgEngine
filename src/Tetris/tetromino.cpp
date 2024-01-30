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
                
            case TetrominoType::L:
            {
                return "J";
                break;
            }

            case TetrominoType::J:
            {
                return "L";
                break;
            }

            case TetrominoType::Z:
            {
                return "S";
                break;
            }

            case TetrominoType::S:
            {
                return "Z";
                break;
            }

            case TetrominoType::T:
            {
                return "T";
                break;
            }

            case TetrominoType::NOMINO:
            {
                return "Empty";
                break;
            }

            default:
            {
                return "Empty";
            }
        }

        return "Empty";
    }
}

Tetromino::Tetromino(const TetrominoType& type) : type(type)
{
    
}

void Tetromino::setMino(const TetrominoType& type)
{
    this->type = type;

    // Set rotation to 0 to always be the correct block
    rotation = 0;

    posX = 4;
    posY = 23;

    switch (type)
    {
        case TetrominoType::I:
        {
            nbPossibleRotation = 2;

            uint8_t rota0[4][4] = {{1, 0, 0, 0},
                                   {1, 0, 0, 0},
                                   {1, 0, 0, 0},
                                   {1, 0, 0, 0}};

            possibleRotation[0] = rota0;

            uint8_t rota1[4][4] = {{0, 0, 0, 0},
                                   {0, 0, 0, 0},
                                   {0, 0, 0, 0},
                                   {1, 1, 1, 1}};

            possibleRotation[1] = rota1;
            break;
        }
        
        case TetrominoType::O:
        {
            nbPossibleRotation = 1;

            uint8_t rota0[4][4] = {{0, 0, 0, 0},
                                   {0, 0, 0, 0},
                                   {1, 1, 0, 0},
                                   {1, 1, 0, 0}};

            possibleRotation[0] = rota0;
            break;
        }
            
        case TetrominoType::J:
        {
            nbPossibleRotation = 4;

            uint8_t rota0[4][4] = {{0, 0, 0, 0},
                                   {0, 0, 0, 0},
                                   {1, 0, 0, 0},
                                   {1, 1, 1, 0}};

            possibleRotation[0] = rota0;

            uint8_t rota1[4][4] = {{0, 0, 0, 0},
                                   {0, 1, 0, 0},
                                   {0, 1, 0, 0},
                                   {1, 1, 0, 0}};

            possibleRotation[1] = rota1;

            uint8_t rota2[4][4] = {{0, 0, 0, 0},
                                   {0, 0, 0, 0},
                                   {1, 1, 1, 0},
                                   {0, 0, 1, 0}};

            possibleRotation[2] = rota2;

            uint8_t rota3[4][4] = {{0, 0, 0, 0},
                                   {1, 1, 0, 0},
                                   {1, 0, 0, 0},
                                   {1, 0, 0, 0}};

            possibleRotation[3] = rota3;
            break;
        }

        case TetrominoType::L:
        {
            nbPossibleRotation = 4;

            uint8_t rota0[4][4] = {{0, 0, 0, 0},
                                   {0, 0, 0, 0},
                                   {0, 0, 1, 0},
                                   {1, 1, 1, 0}};

            possibleRotation[0] = rota0;

            uint8_t rota1[4][4] = {{0, 0, 0, 0},
                                   {1, 1, 0, 0},
                                   {0, 1, 0, 0},
                                   {0, 1, 0, 0}};

            possibleRotation[1] = rota1;

            uint8_t rota2[4][4] = {{0, 0, 0, 0},
                                   {0, 0, 0, 0},
                                   {1, 1, 1, 0},
                                   {1, 0, 0, 0}};

            possibleRotation[2] = rota2;

            uint8_t rota3[4][4] = {{0, 0, 0, 0},
                                   {1, 0, 0, 0},
                                   {1, 0, 0, 0},
                                   {1, 1, 0, 0}};

            possibleRotation[3] = rota3;
            break;
        }

        case TetrominoType::S:
        {
            nbPossibleRotation = 2;

            uint8_t rota0[4][4] = {{0, 0, 0, 0},
                                   {0, 0, 0, 0},
                                   {1, 1, 0, 0},
                                   {0, 1, 1, 0}};

            possibleRotation[0] = rota0;

            uint8_t rota1[4][4] = {{0, 0, 0, 0},
                                   {0, 1, 0, 0},
                                   {1, 1, 0, 0},
                                   {1, 0, 0, 0}};

            possibleRotation[1] = rota1;
            break;
        }

        case TetrominoType::Z:
        {
            nbPossibleRotation = 2;

            uint8_t rota0[4][4] = {{0, 0, 0, 0},
                                   {0, 0, 0, 0},
                                   {0, 1, 1, 0},
                                   {1, 1, 0, 0}};

            possibleRotation[0] = rota0;

            uint8_t rota1[4][4] = {{0, 0, 0, 0},
                                   {1, 0, 0, 0},
                                   {1, 1, 0, 0},
                                   {0, 1, 0, 0}};

            possibleRotation[1] = rota1;
            break;
        }

        case TetrominoType::T:
        {
            nbPossibleRotation = 4;

            uint8_t rota0[4][4] = {{0, 0, 0, 0},
                                   {0, 0, 0, 0},
                                   {0, 1, 0, 0},
                                   {1, 1, 1, 0}};

            possibleRotation[0] = rota0;

            uint8_t rota1[4][4] = {{0, 0, 0, 0},
                                   {0, 1, 0, 0},
                                   {1, 1, 0, 0},
                                   {0, 1, 0, 0}};

            possibleRotation[1] = rota1;

            uint8_t rota2[4][4] = {{0, 0, 0, 0},
                                   {0, 0, 0, 0},
                                   {1, 1, 1, 0},
                                   {0, 1, 0, 0}};

            possibleRotation[2] = rota2;

            uint8_t rota3[4][4] = {{0, 0, 0, 0},
                                   {1, 0, 0, 0},
                                   {1, 1, 0, 0},
                                   {1, 0, 0, 0}};

            possibleRotation[3] = rota3;
            break;
        }
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
    // bag1 = {TetrominoType::I, TetrominoType::I, TetrominoType::I, TetrominoType::I, TetrominoType::I, TetrominoType::I, TetrominoType::I};
    // bag2 = {TetrominoType::I, TetrominoType::I, TetrominoType::I, TetrominoType::I, TetrominoType::I, TetrominoType::I, TetrominoType::I};

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

    auto entity2 = ecsRef->createEntity();

    lockTimer = ecsRef->attach<Timer>(entity2);

    lockTimer->interval = 500;

    lockTimer->callback = makeCallable<LockTimeout>();

    lockTimer->running = false;

    auto canvasTex = makeUiTexture(ecsRef, 24 + 24 * 10, 24 + 24 * 20, "Canvas");

    auto canvasUi = canvasTex.get<UiComponent>();

    canvasUi->setZ(0);

    auto nextTex = makeUiTexture(ecsRef, 27 * 16, 40 * 4, "Next");

    auto nextUi = nextTex.get<UiComponent>();

    nextUi->setZ(0);
    nextUi->setLeftAnchor(canvasUi->right);

    auto holdTex = makeUiTexture(ecsRef, 30 * 4, 40 * 4, "Hold");

    auto holdUi = holdTex.get<UiComponent>();

    holdUi->setZ(0);
    holdUi->setLeftAnchor(canvasUi->right);
    holdUi->setTopAnchor(nextUi->bottom);

    for (size_t i = 0; i < 4; i++)
    {
        for (size_t j = 0; j < 4; j++)
        {
            auto tex = makeUiTexture(ecsRef, 24, 24, "Empty");

            auto ui = tex.get<UiComponent>();

            ui->setTopAnchor(holdUi->top);
            ui->setLeftAnchor(holdUi->left);

            ui->setTopMargin(52 + 24 * i);
            ui->setLeftMargin(12 + 24 * j);

            // ui->setX(300 + 24 * i);
            // ui->setY(400 + 24 * j);

            ui->setZ(0);

            holdedCanvas[i][j] = tex.entity;
        }
    }

    for (size_t i = 0; i < 10; i++)
    {
        for (size_t j = 0; j < 24; j++)
        {
            auto tex = makeUiTexture(ecsRef, 24, 24, "Empty");

            auto ui = tex.get<UiComponent>();

            ui->setX(12 + 24 * i);
            ui->setY(-12 + 20 * 24 - 24 * j);
            ui->setZ(0);

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
            if (currentMino.possibleRotation[currentMino.rotation].pos[j][i] == 1)
            {
                if (canvas[currentMino.posX + x + i][currentMino.posY + y - j] != -1)
                {
                    move = false;
                }
            }
        }
    }

    if (move)
    {
        currentMino.posX += x;
        currentMino.posY += y;
    }

    std::string movedStr = move ? "YES" : "NO";

    LOG_INFO(DOM, "Move helper moved: " << movedStr);

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
        setMino(-1, "Empty");
        move = moveHelper(0, -1);
        setMino(static_cast<uint8_t>(currentMino.type), tetrominoTypeToString(currentMino.type));
    }

    if (not move)
    {
        startLockTimer();
    }
}

void GameCanvas::moveLeft()
{
    if (currentMino.posX > 0)
    {
        setMino(-1, "Empty");
        moveHelper(-1, 0);
        setMino(static_cast<uint8_t>(currentMino.type), tetrominoTypeToString(currentMino.type));
    }

    resetLockTimer();
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
        setMino(-1, "Empty");
        move = moveHelper(0, 1);
        setMino(static_cast<uint8_t>(currentMino.type), tetrominoTypeToString(currentMino.type));
    }

    if (not move)
    {
        startLockTimer();
    }
}

void GameCanvas::moveRight()
{
    if (not ((currentMino.type == TetrominoType::I and currentMino.rotation == 1) and currentMino.posX > 5) and not
           (((currentMino.type == TetrominoType::S and currentMino.rotation == 0) or
             (currentMino.type == TetrominoType::Z and currentMino.rotation == 0) or
             (currentMino.type == TetrominoType::T and currentMino.rotation == 0) or
             (currentMino.type == TetrominoType::T and currentMino.rotation == 2) or
             (currentMino.type == TetrominoType::J and currentMino.rotation == 0) or
             (currentMino.type == TetrominoType::J and currentMino.rotation == 2) or
             (currentMino.type == TetrominoType::L and currentMino.rotation == 0) or
             (currentMino.type == TetrominoType::L and currentMino.rotation == 2)) and currentMino.posX > 6) and not
            ((currentMino.type == TetrominoType::O or
             (currentMino.type == TetrominoType::S and currentMino.rotation == 1) or
             (currentMino.type == TetrominoType::Z and currentMino.rotation == 1) or
             (currentMino.type == TetrominoType::T and currentMino.rotation == 1) or
             (currentMino.type == TetrominoType::T and currentMino.rotation == 3) or
             (currentMino.type == TetrominoType::J and currentMino.rotation == 1) or
             (currentMino.type == TetrominoType::J and currentMino.rotation == 3) or
             (currentMino.type == TetrominoType::L and currentMino.rotation == 1) or
             (currentMino.type == TetrominoType::L and currentMino.rotation == 3)) and currentMino.posX > 7) and not
            ((currentMino.type == TetrominoType::I and currentMino.rotation == 0) and currentMino.posX > 8))
    {
        setMino(-1, "Empty");
        moveHelper(1, 0);
        setMino(static_cast<uint8_t>(currentMino.type), tetrominoTypeToString(currentMino.type));
    }

    resetLockTimer();
}

void GameCanvas::snapBottom()
{
    bool move = true;

    setMino(-1, "Empty");

    do
    {
        if (currentMino.posY < 4)
        {
            move = false;
        }
        else
        {
            
            move = moveHelper(0, -1);
            
        }
    } while(move);

    setMino(static_cast<uint8_t>(currentMino.type), tetrominoTypeToString(currentMino.type));

    onEvent(LockTimeout{});
}

void GameCanvas::rotate()
{
    setMino(-1, "Empty");

    bool placeable = false;

    currentMino.rotation = (currentMino.rotation + 1) % currentMino.nbPossibleRotation;

    int offsetX = 0;

    if ((currentMino.type == TetrominoType::I and currentMino.rotation == 1) and currentMino.posX == 9)
    {
        offsetX = -3;
    }
    else if ((currentMino.type == TetrominoType::I and currentMino.rotation == 1) and currentMino.posX == 8)
    {
        offsetX = -2;
    }
    else if ((currentMino.type == TetrominoType::I and currentMino.rotation == 1) and currentMino.posX == 7)
    {
        offsetX = -1;
    }
    else if(((currentMino.type == TetrominoType::S and currentMino.rotation == 0) or
             (currentMino.type == TetrominoType::Z and currentMino.rotation == 0) or
             (currentMino.type == TetrominoType::T and currentMino.rotation == 0) or
             (currentMino.type == TetrominoType::T and currentMino.rotation == 2) or
             (currentMino.type == TetrominoType::J and currentMino.rotation == 0) or
             (currentMino.type == TetrominoType::J and currentMino.rotation == 2) or
             (currentMino.type == TetrominoType::L and currentMino.rotation == 0) or
             (currentMino.type == TetrominoType::L and currentMino.rotation == 2)) and currentMino.posX > 7)
    {
        offsetX = -1;
    }

    placeable = moveHelper(offsetX, 0);

    if (not placeable)
        currentMino.posX += offsetX;

    while (not placeable and currentMino.posY < 20)
    {
        placeable = moveHelper(0, 1);

        if (not placeable)
            currentMino.posY++;
    }

    setMino(static_cast<uint8_t>(currentMino.type), tetrominoTypeToString(currentMino.type));

    resetLockTimer();
}

void GameCanvas::hold()
{
    if (swapped)
    {
        return;
    }

    setMino(-1, "Empty");

    setHoldedMino(-1, "Empty");

    if (holdedMino.type == TetrominoType::NOMINO)
    {
        holdedMino.setMino(currentMino.type);

        spawnTetromino();

        showNext();
    }
    else
    {
        auto temp = currentMino.type;
        currentMino.setMino(holdedMino.type);

        holdedMino.setMino(temp);

        swapped = true;
    }

    setHoldedMino(static_cast<uint8_t>(holdedMino.type), tetrominoTypeToString(holdedMino.type));

    setMino(static_cast<uint8_t>(currentMino.type), tetrominoTypeToString(currentMino.type));
}

void GameCanvas::setMino(int value, const std::string& texture)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        for (uint8_t j = 0; j < 4; j++)
        {
            if (currentMino.possibleRotation[currentMino.rotation].pos[j][i] == 1)
            {
                canvas[currentMino.posX + i][currentMino.posY - j] = value;
                entityCanvas[currentMino.posX + i][currentMino.posY - j]->get<Texture2DComponent>()->setTexture(texture);
            }
        }
    }
}

void GameCanvas::setHoldedMino(int value, const std::string& texture)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        for (uint8_t j = 0; j < 4; j++)
        {
            if (holdedMino.possibleRotation[holdedMino.rotation].pos[j][i] == 1)
            {
                holdedCanvas[i][j]->get<Texture2DComponent>()->setTexture(texture);
            }
        }
    }
}

void GameCanvas::startLockTimer()
{
    lockTimer->currentTime = 0;

    lockTimer->running = true;
}

void GameCanvas::resetLockTimer()
{
    lockTimer->currentTime = 0;

    lockTimer->running = false;
}

void GameCanvas::onEvent(const OnSDLGamepadPressed& event)
{
    if (event.button == SDL_CONTROLLER_BUTTON_START)
    {
        LOG_INFO(DOM, "Game canvas Start Pressed");

        if (not running)
        {
            running = true;

            spawnTetromino();
        }
    }

    if (event.button == SDL_CONTROLLER_BUTTON_X)
    {
        LOG_INFO(DOM, "Game canvas X Pressed");

        hold();
    }


    if (event.button == SDL_CONTROLLER_BUTTON_Y)
    {
        LOG_INFO(DOM, "Game canvas Y Pressed");

        printCanvas();
    }


    if (event.button == SDL_CONTROLLER_BUTTON_B)
    {
        LOG_INFO(DOM, "Game canvas B Pressed");

        rotate();
    }

    if (event.button == SDL_CONTROLLER_BUTTON_DPAD_UP)
    {
        LOG_INFO(DOM, "Game canvas Up Pressed");

        snapBottom();
    }

    if (event.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN)
    {
        LOG_INFO(DOM, "Game canvas Down Pressed");

        moveDown();
    }

    if (event.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT)
    {
        LOG_INFO(DOM, "Game canvas Left Pressed");

        moveLeft();
    }

    if (event.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT)
    {
        LOG_INFO(DOM, "Game canvas Right Pressed");

        moveRight();
    }
}

void GameCanvas::onEvent(const FallTimeout& event)
{
    LOG_INFO(DOM, "Fall timeout: ");

    moveDown();
}

void GameCanvas::onEvent(const LockTimeout& event)
{
    LOG_INFO(DOM, "Lock timeout: ");

    lockTimer->running = false;

    checkClearedLines();

    swapped = false;

    spawnTetromino();

    showNext();
}

void GameCanvas::dropOneLine(uint8_t y, uint8_t nb)
{
    LOG_INFO(DOM, "Game canvas drop one line, line " << y << " becoming " << y + 1 << " and moving: " << nb << " lines");
 
    // If nb = 0 just clear the line
    if (nb == 0)
    {
        for (auto i = 0; i < 10; i++)
        {
            canvas[i][y] = -1;

            auto ent = entityCanvas[i][y];
            ent->get<Texture2DComponent>()->setTexture("Empty");
        }
    }
    else
    {
        for (auto j = 0; j < nb; j++)
        {
            for (auto i = 0; i < 10; i++)
            {
                canvas[i][y + j] = canvas[i][y + j + 1];

                canvas[i][y + j + 1] = -1;

                auto ent = entityCanvas[i][y + j];
                auto ent1 = entityCanvas[i][y + j + 1];

                auto tex = ent1->get<Texture2DComponent>();

                auto texName = tex->textureName;

                ent->get<Texture2DComponent>()->setTexture(texName);
                tex->setTexture("Empty");
            }
        }
    }    
}

void GameCanvas::dropLines(uint8_t y, uint8_t nb)
{
    for (auto j = y; j < 20; j++)
    {
        LOG_INFO(DOM, "Game canvas drop one line, line " << j - nb << " becoming " << j);

        for (auto i = 0; i < 10; i++)
        {
            canvas[i][j - nb] = canvas[i][j];
            canvas[i][j] = -1;

            auto ent = entityCanvas[i][j - nb];
            auto ent1 = entityCanvas[i][j];

            auto tex = ent1->get<Texture2DComponent>();

            ent->get<Texture2DComponent>()->setTexture(tex->textureName);
            tex->setTexture("Empty");
        }
    }
}

void GameCanvas::checkClearedLines()
{
    printCanvas();
    auto yTop = currentMino.posY;

    uint8_t nbDroppedLines = 0;

    for (auto j = 3; j >= 0; j--)
    {
        bool cleared = true;

        for (auto i = 0; i < 10; i++)
        {
            if (canvas[i][yTop - j - nbDroppedLines] == -1)
                cleared = false;
        }

        if (cleared)
        {
            LOG_INFO(DOM, "-------------------------------------");
            LOG_INFO(DOM, "Line " << yTop - j - nbDroppedLines << " cleared");

            dropOneLine(yTop - j - nbDroppedLines, j);

            nbDroppedLines++;
        };
    }

    if (nbDroppedLines > 0)
    {
        LOG_INFO(DOM, "Cleared lines: " << nbDroppedLines);
        
        dropLines(yTop + 1, nbDroppedLines);
    }

    printCanvas();
}

void GameCanvas::showNext()
{

}

void GameCanvas::spawnTetromino()
{
    LOG_INFO(DOM, "Generate Tetromino");

    auto type = generator.generateTetromino();

    auto ent = entityCanvas[5][5];

    // ecsRef->sendEvent(TextureChangeEvent{ent->id, "Ghost", "I"});    

    LOG_INFO(DOM, "Got entity: " << ent->id << " but tex comp has id: " << ent->get<Texture2DComponent>().entityId);

    auto comp = ent->get<Texture2DComponent>();

    LOG_INFO(DOM, "Entity in comp: " << comp.entityId << " tex: " << comp->entityId);

    if (ent->get<Texture2DComponent>()->ecsRef)
    {
        LOG_INFO(DOM, "Tex entity is correctly loaded");
    }

    // ent->get<Texture2DComponent>()->setTexture("I");

    LOG_INFO(DOM, "New Tetromino: " << static_cast<uint8_t>(type));

    currentMino.setMino(type);

    LOG_INFO(DOM, "New Tetromino: " << static_cast<uint8_t>(currentMino.type));

    for (auto i = 0; i < 4; i++)
    {
        for (auto j = 0; j < 4; j++)
        {
            if (currentMino.possibleRotation[currentMino.rotation].pos[j][i] != 0)
            {
                canvas[currentMino.posX + i][currentMino.posY - j] = static_cast<uint8_t>(currentMino.type);
                auto ent = entityCanvas[currentMino.posX + i][currentMino.posY - j];

                ent->get<Texture2DComponent>()->setTexture(tetrominoTypeToString(currentMino.type));
            }
        }
    }
}

void GameCanvas::printCanvas()
{
    std::string line = "";

    for (auto i = 23; i >= 0; i--)
    {
        if (i >= 10)
            line = std::to_string(i) + ": | ";
        else
            line = " " + std::to_string(i) + ": | ";

        for (auto j = 0; j < 10; j++)
        {
            if (canvas[j][i] == -1)
            {
                line += "-1 |";
            }
            else
            {
                line += " " + std::to_string(canvas[j][i]) + " |";
            }
        }

        LOG_INFO(DOM, line);
    }
}