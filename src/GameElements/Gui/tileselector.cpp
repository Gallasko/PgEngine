#include "tileselector.h"

#include "UI/texture.h"

namespace pg
{
    template <>
    void renderer(MasterRenderer* masterRenderer, TileSelector* tileSelector)
    {
        if(tileSelector->visible == false)
            return;

        //texture->setX(this->x);
        //texture->setY(this->y);
        
        auto rTable = masterRenderer->getParameter();

        const int screenWidth = rTable["ScreenWidth"];
        const int screenHeight = rTable["ScreenHeight"];

        QMatrix4x4 projection;
        QMatrix4x4 view;
        QMatrix4x4 model;
        QMatrix4x4 scale;

        masterRenderer->render(tileSelector->texture);

        auto defaultShaderProgram = masterRenderer->getShader("default");
        defaultShaderProgram->bind();
        
        //glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, masterRenderer->getTexture("atlas"));

        auto tileWidth = 100.0f, tileHeight = 100.0f;

        scale.setToIdentity();
        scale.scale(QVector3D(tileWidth / screenWidth, tileHeight / screenHeight, 0.0f));
        defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("scale"), scale);

        for(auto tile : tileSelector->tileRendererVector)
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
        masterRenderer->render(tileSelector->textVector);
    }
}

TileSelector::TileSelector(Map *map, TilesLoader *tileLoader, FontLoader *fontLoader, UiComponent*) : map(map), tileLoader(tileLoader), fontLoader(fontLoader)
{
    this->width = 240;
    this->height = 135;

    texture = new TextureComponent(this->width, this->height, "Menu2");
    texture->pos.x = &this->pos.x;
    texture->pos.y = &this->pos.y;

    auto text1 = new Sentence({"Base House"}, 4.0f, fontLoader);

    textVector.push_back(text1); // TODO take care of the pb of object referencing because dumb std::vector create copies and doesn t not move the object in it 
    //textVector[0].setBottomAnchor(&this->bottom); //TODO Fix issue when text is rendered in bottom right it need to be divided by 2
    //textVector[0].setBottomAnchor(&this->bottom);

    textVector[0]->setTopAnchor(texture->top); // TODO need to create a check of fix this issue: parenting must be done AFTER pushing the object in the vector
                                            // cause the vector create a copy thus invaliding parent to child call !
    textVector[0]->setTopMargin(41);
    textVector[0]->setLeftAnchor(texture->left);
    textVector[0]->setLeftMargin(10);

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

    tileRendererVector[0].setTopAnchor(textVector[0]->bottom);
    tileRendererVector[0].setLeftAnchor(textVector[0]->left);
    tileRendererVector[0].setTopMargin(5);

    tileRendererVector[0].pos.z = &this->pos.z;

    tileRendererVector[0].setWidth(50.0f);
    tileRendererVector[0].setHeight(50.0f);

    tileRendererVector[1].setTopAnchor(tileRendererVector[0].top);
    tileRendererVector[1].setLeftAnchor(tileRendererVector[0].right);
    tileRendererVector[1].setLeftMargin(5);

    tileRendererVector[1].pos.z = &this->pos.z;

    tileRendererVector[1].setWidth(50.0f);
    tileRendererVector[1].setHeight(50.0f);

    tileRendererVector[2].setTopAnchor(tileRendererVector[1].top);
    tileRendererVector[2].setLeftAnchor(tileRendererVector[1].right);
    tileRendererVector[2].setLeftMargin(5);

    tileRendererVector[2].pos.z = &this->pos.z;

    tileRendererVector[2].setWidth(50.0f);
    tileRendererVector[2].setHeight(50.0f);
    
    // for(int i = 0; i < 3; i++)
        // mouseAreaVector.push_back(makeMouseArea(&tileRendererVector[i], map, Map::changeTile, nullptr, tileRendererVector[i].id));

    this->visible = true;
}

TileSelector::~TileSelector()
{
}

void TileSelector::setVisibility(bool visibility)
{
    visible = visibility;
}

//void TileSelector::render(unsigned int screenWidth, unsigned int screenHeight, QOpenGLShaderProgram* defaultShaderProgram, unsigned int tileTexture, QOpenGLShaderProgram* textShaderProgram, unsigned int fontTexture, qint64 currentTime)
void TileSelector::render(MasterRenderer* masterRenderer)
{ 
    renderer(masterRenderer, this); 
}