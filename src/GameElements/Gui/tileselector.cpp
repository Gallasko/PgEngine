#include "tileselector.h"

TileSelector::TileSelector(Map *map, TilesLoader *tileLoader, FontLoader *fontLoader, UiComponent* screenUi) : map(map), tileLoader(tileLoader), fontLoader(fontLoader)
{
    initializeOpenGLFunctions();

    this->width = 240;
    this->height = 135;

    this->setX(400);
    this->setY(345);

    texture = new TextureComponent(this->width, this->height, "res/menu/Menu2.png");
    texture->setX(this->x);
    texture->setY(this->y);

    auto text1 = Sentence({"Base House"}, 4.0f, fontLoader);
    //text1.setX(this->x + 10);
    //text1.setY(this->y + 10);
    text1.setX(this->x + 11);
    text1.setY(this->y + 41);

    textVector.push_back(text1);

    auto tileRenderer = LoaderRenderComponent<TilesLoader::TilesId>(tileLoader->getTile("Base House"));
    tileRenderer.setX(this->x + 11);
    tileRenderer.setY(this->y + 41 + 30);
    tileRenderer.setWidth(50.0f);
    tileRenderer.setHeight(50.0f);

    auto tileRenderer2 = LoaderRenderComponent<TilesLoader::TilesId>(tileLoader->getTile("Base Shop"));
    tileRenderer2.setX(this->x + 11 + 60);
    tileRenderer2.setY(this->y + 41 + 30);
    tileRenderer2.setWidth(50.0f);
    tileRenderer2.setHeight(50.0f);

    auto tileRenderer3 = LoaderRenderComponent<TilesLoader::TilesId>(tileLoader->getTile("Base Road RoundAbout"));
    tileRenderer3.setX(this->x + 11 + 60 + 60);
    tileRenderer3.setY(this->y + 41 + 30);
    tileRenderer3.setWidth(50.0f);
    tileRenderer3.setHeight(50.0f);
    
    tileRendererVector.push_back(tileRenderer);
    tileRendererVector.push_back(tileRenderer2);
    tileRendererVector.push_back(tileRenderer3);

    mouseAreaVector.push_back(new MouseInputBase<Map>(&tileRendererVector[0]));
    mouseAreaVector.push_back(new MouseInputBase<Map>(&tileRendererVector[1]));
    mouseAreaVector.push_back(new MouseInputBase<Map>(&tileRendererVector[2]));

    mouseAreaVector[0]->registerFunc(&map->changeTile, map);
    mouseAreaVector[1]->registerFunc(&map->changeTile, map);
    mouseAreaVector[2]->registerFunc(&map->changeTile, map);

    this->visible = true;
    this->scale = 1.0;
}

void TileSelector::mouseInput(Input* inputHandler, double deltaTime)
{
    auto mousePos = inputHandler->getMousePos();

    //std::cout << mousePos.x() << ", " << mousePos.y() << std::endl;

    if(inputHandler->isButtonPressed(Qt::LeftButton))
    {
        for(unsigned int i = 0; i < mouseAreaVector.size(); i++)
        {
            auto mouseArea = mouseAreaVector[i];
            //std::cout << "Mouse Hovering: " << *mouseArea->x << ", " << *mouseArea->y << ", " << *mouseArea->width << ", " << *mouseArea->height << std::endl;
            
            if(mousePos.x() > *mouseArea->x / static_cast<int>(mouseArea->scale) && mousePos.x() < (*mouseArea->x + *mouseArea->width) / static_cast<int>(mouseArea->scale) && mousePos.y() < (*mouseArea->y + *mouseArea->height) / static_cast<int>(mouseArea->scale) && mousePos.y() > *mouseArea->y / static_cast<int>(mouseArea->scale) && *mouseArea->enable)
            {
                //std::cout << "Mouse Hovering: " << *mouseArea->x << ", " << *mouseArea->y << ", " << *mouseArea->width << ", " << *mouseArea->height << std::endl;
                mouseArea->call(inputHandler, deltaTime, tileRendererVector[i].id);
            }
        }
    }
}

void TileSelector::setVisibility(bool visibility)
{
    visible = visibility;
}

//void TileSelector::render(unsigned int screenWidth, unsigned int screenHeight, QOpenGLShaderProgram* defaultShaderProgram, unsigned int tileTexture, QOpenGLShaderProgram* textShaderProgram, unsigned int fontTexture, qint64 currentTime)
void TileSelector::render(RefracRef rTable, ShaderRef sTable, TextureRef tTable)
{
    const int screenWidth = rTable["ScreenWidth"];
    const int screenHeight = rTable["ScreenHeight"];
    const int currentTime = rTable["CurrentTime"];

    if(this->visible == false)
        return;

    QMatrix4x4 projection;
    QMatrix4x4 view;
    QMatrix4x4 model;
    QMatrix4x4 scale;

    projection.setToIdentity();
    model.setToIdentity();
    scale.setToIdentity();
    scale.scale(QVector3D(2.0f / screenWidth, 2.0f / screenHeight, 0.0f));

    // Text rendering
    auto defaultShaderProgram = sTable["default"];

    defaultShaderProgram->bind();

    defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("projection"), projection);
    defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("model"), model);
    defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("scale"), scale);

    //gl scissor for list views 
    //glEnable(GL_SCISSOR_TEST);
    //glScissor(300, 200, 200, 500);

    texture->generateMesh();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->texture);

    view.setToIdentity();
    view.translate(QVector3D(-1.0f + 2.0f * (float)(texture->x) / screenWidth, 1.0f + 2.0f * (float)( -texture->y) / screenHeight, 0.0f));

    defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("view"), view);

    texture->VAO->bind();
    glDrawElements(GL_TRIANGLES, texture->modelInfo.nbIndices, GL_UNSIGNED_INT, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tTable["atlas"]);

    auto tileWidth = 100.0f, tileHeight = 100.0f;

    scale.setToIdentity();
    scale.scale(QVector3D(tileWidth / screenWidth, tileHeight / screenHeight, 0.0f));
    defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("scale"), scale);

    for(auto tile : tileRendererVector)
    {
        view.setToIdentity();
        view.translate(QVector3D(-1.0f + 2.0f * (float)(tile.x + (tileHeight / 4.0f)) / screenWidth, 1.0f + 2.0f * (float)( -tile.y - (tileHeight / 8.0f)) / screenHeight, 0.0f));

        defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("view"), view);

        tile.id->getMesh()->bind();
        glDrawElements(GL_TRIANGLES, tile.id->getModelInfo().nbIndices, GL_UNSIGNED_INT, 0);
    }

    //glDisable(GL_SCISSOR_TEST);

    defaultShaderProgram->release();

    auto textShaderProgram = sTable["text"];

    textShaderProgram->bind();

    scale.setToIdentity();
    scale.scale(QVector3D(1.0f / screenWidth, 1.0f / screenHeight, 0.0f));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tTable["font"]);

    textShaderProgram->setUniformValue(textShaderProgram->uniformLocation("projection"), projection);
    textShaderProgram->setUniformValue(textShaderProgram->uniformLocation("model"), model);
    textShaderProgram->setUniformValue(textShaderProgram->uniformLocation("scale"), scale);

    textShaderProgram->setUniformValue(textShaderProgram->uniformLocation("time"), static_cast<int>(currentTime % 314159));

    for(auto text : textVector)
    {
        if(text.initialised == false)
            text.generateMesh();

        view.setToIdentity();
        view.translate(QVector3D(-1.0f + 2.0 * (float)(text.x) / screenWidth, 1.0f + 2.0 * (float)( -text.y) / screenHeight, 0.0f));
        textShaderProgram->setUniformValue(textShaderProgram->uniformLocation("view"), view);

        text.VAO->bind();
        glDrawElements(GL_TRIANGLES, text.modelInfo.nbIndices, GL_UNSIGNED_INT, 0);

        textShaderProgram->release();   
    }
}

TileSelector::~TileSelector()
{

}