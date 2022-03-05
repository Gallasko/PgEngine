#include "sentencesystem.h"

namespace pg
{
    template<>
    void serialize(Archive& archive, const SentenceText& value)
    {
        archive.startSerialization("Sentence Text");

        //TODO put all the variable to be serialized
        serialize(archive, "text", value.text);
        serialize(archive, "main color", value.mainColor);
        serialize(archive, "outline1", value.outline1);
        serialize(archive, "outline2", value.outline2);

        archive.endSerialization();
    }

    template<>
    void serialize(Archive& archive, const Sentence& value)
    {
        archive.startSerialization("Sentence");

        serialize(archive, value.modelInfo);
        serialize(archive, "Nb chara", value.nbChara);

        serialize(archive, value.text);

        archive.endSerialization();
    }

    template<>
    void renderer(MasterRenderer* masterRenderer, Sentence* sentence)
    { 
        auto rTable = masterRenderer->getParameter();
        const int screenWidth = rTable["ScreenWidth"];
        const int screenHeight = rTable["ScreenHeight"];
        const int currentTime = rTable["CurrentTime"];

        QMatrix4x4 projection;
        QMatrix4x4 view;
        QMatrix4x4 model;
        QMatrix4x4 scale;

        projection.setToIdentity();
        model.setToIdentity();
        scale.setToIdentity();
        scale.scale(QVector3D(2.0f / screenWidth, 2.0f / screenHeight, 0.0f));

        auto shaderProgram = masterRenderer->getShader("text");

        // Text rendering
        
        shaderProgram->bind();

        shaderProgram->setUniformValue(shaderProgram->uniformLocation("projection"), projection);
        shaderProgram->setUniformValue(shaderProgram->uniformLocation("model"), model);
        shaderProgram->setUniformValue(shaderProgram->uniformLocation("scale"), scale);

        shaderProgram->setUniformValue(shaderProgram->uniformLocation("time"), static_cast<int>(currentTime % 314159));

        if(sentence->initialised == false)
            sentence->generateMesh();

        //TODO store texture of the font in the font loader to get it back in the renderer
        //glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, masterRenderer->getTexture("font"));

        view.setToIdentity();
        view.translate(QVector3D(-1.0f + 2.0f * (float)(sentence->pos.x) / screenWidth, 1.0f + 2.0f * (float)( -sentence->pos.y) / screenHeight, 0.0f));

        shaderProgram->setUniformValue(shaderProgram->uniformLocation("view"), view);

        sentence->VAO->bind();
        glDrawElements(GL_TRIANGLES, sentence->modelInfo.nbIndices, GL_UNSIGNED_INT, 0);

        shaderProgram->release();
    }

    template<>
    void renderer(MasterRenderer* masterRenderer, std::vector<Sentence*> sentenceList)
    { 
        auto rTable = masterRenderer->getParameter();
        const int screenWidth = rTable["ScreenWidth"];
        const int screenHeight = rTable["ScreenHeight"];
        const int currentTime = rTable["CurrentTime"];

        QMatrix4x4 projection;
        QMatrix4x4 view;
        QMatrix4x4 model;
        QMatrix4x4 scale;

        projection.setToIdentity();
        model.setToIdentity();
        scale.setToIdentity();
        scale.scale(QVector3D(1.0f / screenWidth, 1.0f / screenHeight, 0.0f));

        auto shaderProgram = masterRenderer->getShader("text");

        // Tex rendering
        
        shaderProgram->bind();

        shaderProgram->setUniformValue(shaderProgram->uniformLocation("projection"), projection);
        shaderProgram->setUniformValue(shaderProgram->uniformLocation("model"), model);
        shaderProgram->setUniformValue(shaderProgram->uniformLocation("scale"), scale);

        shaderProgram->setUniformValue(shaderProgram->uniformLocation("time"), static_cast<int>(currentTime % 314159));
        //TODO store texture of the font in the font loader to get it back in the renderer
        glBindTexture(GL_TEXTURE_2D, masterRenderer->getTexture("font"));

        for(auto& sentence : sentenceList)
        {
            if(sentence->initialised == false)
            sentence->generateMesh();

            view.setToIdentity();
            view.translate(QVector3D(-1.0f + 2.0f * (float)(sentence->pos.x) / screenWidth, 1.0f + 2.0f * (float)( -sentence->pos.y) / screenHeight, 0.0f));

            shaderProgram->setUniformValue(shaderProgram->uniformLocation("view"), view);

            sentence->VAO->bind();
            glDrawElements(GL_TRIANGLES, sentence->modelInfo.nbIndices, GL_UNSIGNED_INT, 0);
        }

        shaderProgram->release();
    }

    //TODO make a class responsible for creating all the VAO/VBO for the meshes so the class doesn t need to subclass QOpenGLFunctions

    Sentence::Sentence(const SentenceText& sentence, const float& scale, FontLoader *font) : QOpenGLFunctions(), scale(scale), font(font)
    {
        initializeOpenGLFunctions(); 

        VAO = new QOpenGLVertexArrayObject();
        VBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        EBO = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);

        VAO->create();
        VBO->create();
        EBO->create();
        
        setText(sentence, font);
    }

    Sentence::Sentence(const Sentence &rhs) : UiComponent(rhs), QOpenGLFunctions()
    {
        initializeOpenGLFunctions(); 

        VAO = new QOpenGLVertexArrayObject();
        VBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        EBO = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);

        VAO->create();
        VBO->create();
        EBO->create();

        this->text = rhs.text;
        this->nbChara = rhs.nbChara;
        this->scale = rhs.scale;

        this->modelInfo = rhs.modelInfo;

        this->font = rhs.font;
        setText(text, font);

        update();
    }

    Sentence::~Sentence()
    {
        delete VAO;
        delete VBO;
        delete EBO;
    }

    void Sentence::setText(const SentenceText& sentence, FontLoader *font)
    {
        if(text != sentence)
        {
            nbChara = sentence.text.length();
            FontLoader::Font* letter;
            
            modelInfo.nbVertices = 72 * nbChara;
            modelInfo.nbIndices = 6 * nbChara;

            if(modelInfo.vertices != nullptr)
                delete[] modelInfo.vertices;
            if(modelInfo.indices != nullptr)
                delete[] modelInfo.indices;

            modelInfo.vertices = new float [modelInfo.nbVertices];
            modelInfo.indices = new unsigned int [modelInfo.nbIndices];

            int currentX = 0.0f;
            int outO = 0.0f; //Outline Offset

            constant::ModelInfo letterModel; 

            for(int i = 0; i < nbChara; i++)
            {
                letter = font->getChara(std::string(1, sentence.text.at(i)));

                outO = 0.0f;

                auto w = letter->getWidth() * scale;
                auto h = letter->getHeight() * scale;
                auto o = letter->getOffset() * scale;

                if(sentence.outline1.w == 0.0f)
                    outO += 1.0 * scale;

                if(sentence.outline2.w == 0.0f)
                    outO += 1.0 * scale;

                letterModel = letter->getModelInfo();

                // Coord
                modelInfo.vertices[i * 72 + 0]  = currentX - outO    ; modelInfo.vertices[i * 72 + 1]  =     -o + outO; modelInfo.vertices[i * 72 + 2]  = 0.0f;
                modelInfo.vertices[i * 72 + 18] = currentX + w + outO; modelInfo.vertices[i * 72 + 19] =     -o + outO; modelInfo.vertices[i * 72 + 20] = 0.0f;
                modelInfo.vertices[i * 72 + 36] = currentX - outO    ; modelInfo.vertices[i * 72 + 37] = -h - o - outO; modelInfo.vertices[i * 72 + 38] = 0.0f;
                modelInfo.vertices[i * 72 + 54] = currentX + w + outO; modelInfo.vertices[i * 72 + 55] = -h - o - outO; modelInfo.vertices[i * 72 + 56] = 0.0f;

                // Tex Coord
                modelInfo.vertices[i * 72 + 3]  = letterModel.vertices[3]  - outO / font->getAtlasWidth(); modelInfo.vertices[i * 72 + 4]  = letterModel.vertices[4]  - outO / font->getAtlasHeight();  
                modelInfo.vertices[i * 72 + 21] = letterModel.vertices[8]  + outO / font->getAtlasWidth(); modelInfo.vertices[i * 72 + 22] = letterModel.vertices[9]  - outO / font->getAtlasHeight();
                modelInfo.vertices[i * 72 + 39] = letterModel.vertices[13] - outO / font->getAtlasWidth(); modelInfo.vertices[i * 72 + 40] = letterModel.vertices[14] + outO / font->getAtlasHeight();
                modelInfo.vertices[i * 72 + 57] = letterModel.vertices[18] + outO / font->getAtlasWidth(); modelInfo.vertices[i * 72 + 58] = letterModel.vertices[19] + outO / font->getAtlasHeight();

                //Main Color
                modelInfo.vertices[i * 72 + 5]  = sentence.mainColor.x; modelInfo.vertices[i * 72 + 6]  = sentence.mainColor.y; modelInfo.vertices[i * 72 + 7]  = sentence.mainColor.z; modelInfo.vertices[i * 72 + 8]  = sentence.mainColor.w;
                modelInfo.vertices[i * 72 + 23] = sentence.mainColor.x; modelInfo.vertices[i * 72 + 24] = sentence.mainColor.y; modelInfo.vertices[i * 72 + 25] = sentence.mainColor.z; modelInfo.vertices[i * 72 + 26] = sentence.mainColor.w;
                modelInfo.vertices[i * 72 + 41] = sentence.mainColor.x; modelInfo.vertices[i * 72 + 42] = sentence.mainColor.y; modelInfo.vertices[i * 72 + 43] = sentence.mainColor.z; modelInfo.vertices[i * 72 + 44] = sentence.mainColor.w;
                modelInfo.vertices[i * 72 + 59] = sentence.mainColor.x; modelInfo.vertices[i * 72 + 60] = sentence.mainColor.y; modelInfo.vertices[i * 72 + 61] = sentence.mainColor.z; modelInfo.vertices[i * 72 + 62] = sentence.mainColor.w;

                //Outline 1
                modelInfo.vertices[i * 72 + 9]  = sentence.outline1.x; modelInfo.vertices[i * 72 + 10] = sentence.outline1.y; modelInfo.vertices[i * 72 + 11] = sentence.outline1.z; modelInfo.vertices[i * 72 + 12] = sentence.outline1.w;
                modelInfo.vertices[i * 72 + 27] = sentence.outline1.x; modelInfo.vertices[i * 72 + 28] = sentence.outline1.y; modelInfo.vertices[i * 72 + 29] = sentence.outline1.z; modelInfo.vertices[i * 72 + 30] = sentence.outline1.w;
                modelInfo.vertices[i * 72 + 45] = sentence.outline1.x; modelInfo.vertices[i * 72 + 46] = sentence.outline1.y; modelInfo.vertices[i * 72 + 47] = sentence.outline1.z; modelInfo.vertices[i * 72 + 48] = sentence.outline1.w;
                modelInfo.vertices[i * 72 + 63] = sentence.outline1.x; modelInfo.vertices[i * 72 + 64] = sentence.outline1.y; modelInfo.vertices[i * 72 + 65] = sentence.outline1.z; modelInfo.vertices[i * 72 + 66] = sentence.outline1.w;

                //Outline 2
                modelInfo.vertices[i * 72 + 13] = sentence.outline2.x; modelInfo.vertices[i * 72 + 14] = sentence.outline2.y; modelInfo.vertices[i * 72 + 15] = sentence.outline2.z; modelInfo.vertices[i * 72 + 16] = sentence.outline2.w;
                modelInfo.vertices[i * 72 + 31] = sentence.outline2.x; modelInfo.vertices[i * 72 + 32] = sentence.outline2.y; modelInfo.vertices[i * 72 + 33] = sentence.outline2.z; modelInfo.vertices[i * 72 + 34] = sentence.outline2.w;
                modelInfo.vertices[i * 72 + 49] = sentence.outline2.x; modelInfo.vertices[i * 72 + 50] = sentence.outline2.y; modelInfo.vertices[i * 72 + 51] = sentence.outline2.z; modelInfo.vertices[i * 72 + 52] = sentence.outline2.w;
                modelInfo.vertices[i * 72 + 67] = sentence.outline2.x; modelInfo.vertices[i * 72 + 68] = sentence.outline2.y; modelInfo.vertices[i * 72 + 69] = sentence.outline2.z; modelInfo.vertices[i * 72 + 70] = sentence.outline2.w;

                //Effect
                modelInfo.vertices[i * 72 + 17] = (float)sentence.effect;
                modelInfo.vertices[i * 72 + 35] = (float)sentence.effect;
                modelInfo.vertices[i * 72 + 53] = (float)sentence.effect;
                modelInfo.vertices[i * 72 + 71] = (float)sentence.effect;

                modelInfo.indices[i * 6 + 0] = 4 * i + 0; modelInfo.indices[i * 6 + 1] = 4 * i + 1; modelInfo.indices[i * 6 + 2] = 4 * i + 2;
                modelInfo.indices[i * 6 + 3] = 4 * i + 1; modelInfo.indices[i * 6 + 4] = 4 * i + 2; modelInfo.indices[i * 6 + 5] = 4 * i + 3;

                currentX += w + 1;

                if(h + o > height)
                    height = h + o;
            }

            width = currentX;

            text = sentence;
            initialised = false;

            update();
        }
        
    }

    //TODO add a parameters to set the usage patern and make it static by default
    void Sentence::generateMesh()
    {
        VAO->bind();

        // position attribute
        VBO->bind();
        VBO->setUsagePattern(QOpenGLBuffer::StaticDraw);
        VBO->allocate(modelInfo.vertices, modelInfo.nbVertices * sizeof(float));

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 18 * sizeof(float), (void*)0);

        // texture coord attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 18 * sizeof(float), (void*)(3 * sizeof(float)));

        // texture coord attribute
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 18 * sizeof(float), (void*)(5 * sizeof(float)));

        // texture coord attribute
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 18 * sizeof(float), (void*)(9 * sizeof(float)));

        // texture coord attribute
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 18 * sizeof(float), (void*)(13 * sizeof(float)));

        // texture coord attribute
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, 18 * sizeof(float), (void*)(17 * sizeof(float)));

        EBO->bind();
        EBO->setUsagePattern(QOpenGLBuffer::StaticDraw);
        EBO->allocate(modelInfo.indices, modelInfo.nbIndices * sizeof(unsigned int));

        VAO->release();

        initialised = true;
    }

    void Sentence::render(MasterRenderer* masterRenderer)
    { 
        renderer(masterRenderer, this); 
    }
}