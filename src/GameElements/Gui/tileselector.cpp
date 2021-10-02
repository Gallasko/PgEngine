#include "tileselector.h"

TileSelector::TileSelector(Map *map, TilesLoader *tileLoader, FontLoader *fontLoader, UiComponent* screenUi) : map(map), tileLoader(tileLoader), fontLoader(fontLoader)
{
    initializeOpenGLFunctions();

    //this->width = 240;
    //this->height = 135;

    //this->setX(400);
    //this->setY(345);
//
    //this->setX(100);
    //this->setY(45);

    std::cout << static_cast<int>(screenUi->right) << std::endl;

    //this->setRightAnchor(&screenUi->right);
    //this->setBottomAnchor(&screenUi->bottom);

    //this->setRightMargin(200);
    //this->setBottomMargin(200);

    //this->setLeftAnchor(&screenUi->left);
    //this->setTopAnchor(&screenUi->top);


    this->setWidth(240);
    this->setHeight(135);

    //std::cout << static_cast<int>(this->bottom) << std::endl;

    
    //this->setX(static_cast<int>(screenUi->width - 240));
    //this->setY(static_cast<int>(screenUi->height - 135));
    
    //this->setLeftMargin(-240);
    
    //this->setTopMargin(-135);

    texture = new TextureComponent(this->width, this->height, "res/menu/Menu2.png");
    texture->pos.x = &this->pos.x;
    texture->pos.y = &this->pos.y;
    
    //texture->setX(this->x);
    //texture->setY(this->y);

    //texture->setTopAnchor(&this->top);
    //this->setTopMargin(-135);
    //texture->setLeftAnchor(&this->left);
    //this->setLeftMargin(-240);

    auto text1 = Sentence({"Base House"}, 2.0f, fontLoader);
    //text1.setX(this->x + 10)
    //text1.setY(this->y + 10);
    
    //text1.setX(this->x + 11);
    //text1.setY(this->y + 41);

    textVector.push_back(text1); // TODO take care of the pb of object referencing because dumb std::vector create copies and doesn t not move the object in it 
    //textVector[0].setBottomAnchor(&this->bottom); //TODO Fix issue when text is rendered in bottom right it need to be divided by 2
    //textVector[0].setBottomAnchor(&this->bottom);

    textVector[0].setTopAnchor(&texture->top); // TODO need to create a check of fix this issue: parenting must be done AFTER pushing the object in the vector
                                            // cause the vector create a copy thus invaliding parent to child call !
    textVector[0].setTopMargin(41);
    textVector[0].setLeftAnchor(&texture->left);
    textVector[0].setLeftMargin(10);

    //textVector[0].setBottomAnchor(&this->bottom); // TODO need to create a check of fix this issue: parenting must be done AFTER pushing the object in the vector
    //                                        // cause the vector create a copy thus invaliding parent to child call !
    //textVector[0].setBottomMargin(5);
    //textVector[0].setRightAnchor(&this->right);
    //textVector[0].setRightMargin(5);

    auto tileRenderer = LoaderRenderComponent<TilesLoader::TilesId>(tileLoader->getTile("Base House"));
    auto tileRenderer2 = LoaderRenderComponent<TilesLoader::TilesId>(tileLoader->getTile("Base Shop"));
    auto tileRenderer3 = LoaderRenderComponent<TilesLoader::TilesId>(tileLoader->getTile("Base Road RoundAbout"));
    
    tileRendererVector.push_back(tileRenderer);
    tileRendererVector.push_back(tileRenderer2);
    tileRendererVector.push_back(tileRenderer3);

    tileRendererVector[0].setTopAnchor(&textVector[0].bottom);
    tileRendererVector[0].setLeftAnchor(&textVector[0].left);
    tileRendererVector[0].setTopMargin(5);

    tileRendererVector[0].setWidth(50.0f);
    tileRendererVector[0].setHeight(50.0f);

    tileRendererVector[1].setTopAnchor(&tileRendererVector[0].top);
    tileRendererVector[1].setLeftAnchor(&tileRendererVector[0].right);
    tileRendererVector[1].setLeftMargin(5);

    tileRendererVector[1].setWidth(50.0f);
    tileRendererVector[1].setHeight(50.0f);

    tileRendererVector[2].setTopAnchor(&tileRendererVector[1].top);
    tileRendererVector[2].setLeftAnchor(&tileRendererVector[1].right);
    tileRendererVector[2].setLeftMargin(5);

    tileRendererVector[2].setWidth(50.0f);
    tileRendererVector[2].setHeight(50.0f);

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
            
            //if(mousePos.x() > *mouseArea->x / static_cast<int>(mouseArea->scale) && mousePos.x() < (*mouseArea->x + *mouseArea->width) / static_cast<int>(mouseArea->scale) && mousePos.y() < (*mouseArea->y + *mouseArea->height) / static_cast<int>(mouseArea->scale) && mousePos.y() > *mouseArea->y / static_cast<int>(mouseArea->scale) && *mouseArea->enable)
            //{
            if(mouseArea->inBound(mousePos.x(), mousePos.y()) && *mouseArea->enable)
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
void TileSelector::render(MasterRenderer* masterRenderer)
{
    if(this->visible == false)
        return;

    //texture->setX(this->x);
    //texture->setY(this->y);
    
    auto rTable = masterRenderer->getParameter();

    const int screenWidth = rTable["ScreenWidth"];
    const int screenHeight = rTable["ScreenHeight"];
    const int currentTime = rTable["CurrentTime"];

    QMatrix4x4 projection;
    QMatrix4x4 view;
    QMatrix4x4 model;
    QMatrix4x4 scale;

    masterRenderer->render<TextureRenderer>(texture);

    auto defaultShaderProgram = masterRenderer->getShader("default");
    defaultShaderProgram->bind();
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, masterRenderer->getTexture("atlas"));

    auto tileWidth = 100.0f, tileHeight = 100.0f;

    scale.setToIdentity();
    scale.scale(QVector3D(tileWidth / screenWidth, tileHeight / screenHeight, 0.0f));
    defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("scale"), scale);

    for(auto tile : tileRendererVector)
    {
        view.setToIdentity();
        view.translate(QVector3D(-1.0f + 2.0f * (float)(tile.pos.x + (tileHeight / 4.0f)) / screenWidth, 1.0f + 2.0f * (float)( -tile.pos.y - (tileHeight / 8.0f)) / screenHeight, 0.0f));

        defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("view"), view);

        tile.id->getMesh()->bind();
        glDrawElements(GL_TRIANGLES, tile.id->getModelInfo().nbIndices, GL_UNSIGNED_INT, 0);
    }

    //glDisable(GL_SCISSOR_TEST);

    defaultShaderProgram->release();

    // TODO create a list rendering
    for(auto& text : textVector)
        masterRenderer->render<SentenceRenderer>(&text);
}

TileSelector::~TileSelector()
{

}