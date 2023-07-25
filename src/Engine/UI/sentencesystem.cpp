#include "Helpers/openglobject.h"

#include "sentencesystem.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

namespace pg
{
    template <>
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

    void SentenceMesh::generateMesh()
    {
        LOG_THIS_MEMBER("Sentence Mesh");

        OpenGLMesh.initialize();

        OpenGLMesh.VAO->bind();

        // position attribute
        OpenGLMesh.VBO->bind();
        OpenGLMesh.VBO->setUsagePattern(OpenGLBuffer::StaticDraw);
        OpenGLMesh.VBO->allocate(modelInfo.vertices, modelInfo.nbVertices * sizeof(float));

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

        OpenGLMesh.EBO->bind();
        OpenGLMesh.EBO->setUsagePattern(OpenGLBuffer::StaticDraw);
        OpenGLMesh.EBO->allocate(modelInfo.indices, modelInfo.nbIndices * sizeof(unsigned int));

        OpenGLMesh.VAO->release();

        initialized = true;
    }

    void SentenceSystem::init()
    {
        auto group = registerGroup<UiComponent, SentenceText>();

        group->addOnGroup([](EntityRef entity) {
            LOG_INFO("Sentence Component System", "Add entity " << entity->id << " to ui - sent group !");

            auto ui = entity->get<UiComponent>();
            auto sentence = entity->get<SentenceText>();

            auto sys = entity->world()->getSystem<SentenceSystem>();

            auto mesh = sys->getSentenceMesh(*sentence, sys->font);

            auto rTex = RenderableTexture{entity->id, ui, mesh};

            std::lock_guard<std::mutex> lock (sys->modificationMutex);

            auto textureId = sys->masterRenderer->getTexture("font");

            sys->tempRenderList[textureId].push_back(rTex);
            
            sys->changed = true;
        });
    }

    void SentenceSystem::onEvent(const OnTextChanged& event)
    {
        auto entity = ecsRef->getEntity(event.entityId);

        auto ui = entity->get<UiComponent>();

        // Todo check if the entity has a sentence text before trying to modify it
        auto sentence = entity->get<SentenceText>();

        sentence->text = event.newText;

        auto sys = entity->world()->getSystem<SentenceSystem>();

        auto mesh = sys->getSentenceMesh(*sentence, font);

        auto rTex = RenderableTexture{event.entityId, ui, mesh};

        LOG_MILE("Sentence Component System", "Modification of id: " << entity->id << " sentence");

        std::lock_guard<std::mutex> lock (sys->modificationMutex);

        auto textureId = sys->masterRenderer->getTexture("font");

        auto first = sys->tempRenderList[textureId].begin();
        auto last = sys->tempRenderList[textureId].end();

        while (first != last)
        {
            if (first->entityId == event.entityId)
            {
                *first = rTex;
                break;
            }

            ++first;
        }

        sys->changed = true;
    }

    void SentenceSystem::onEvent(const UiComponentChangeEvent& event)
    {
        auto entity = ecsRef->getEntity(event.id);

        if(not entity->has<SentenceText>())
            return;

        auto ui = entity->get<UiComponent>();

        // Todo check if the entity has a sentence text before trying to modify it
        auto sentence = entity->get<SentenceText>();

        auto sys = entity->world()->getSystem<SentenceSystem>();

        auto mesh = sys->getSentenceMesh(*sentence, font);

        auto rTex = RenderableTexture{event.id, ui, mesh};

        LOG_MILE("Sentence Component System", "Modification of id: " << entity->id << " sentence");

        std::lock_guard<std::mutex> lock (sys->modificationMutex);

        auto textureId = sys->masterRenderer->getTexture("font");

        auto first = sys->tempRenderList[textureId].begin();
        auto last = sys->tempRenderList[textureId].end();

        while (first != last)
        {
            if (first->entityId == event.id)
            {
                *first = rTex;
                break;
            }

            ++first;
        }

        sys->changed = true;
    }

    void SentenceSystem::render()
    {
        auto rTable = masterRenderer->getParameter();
        const int screenWidth = rTable["ScreenWidth"];
        const int screenHeight = rTable["ScreenHeight"];

        glm::mat4 projection = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 scale = glm::mat4(1.0f);

        scale = glm::scale(scale, glm::vec3(2.0f / screenWidth, 2.0f / screenHeight, 0.0f));

        // TODO why does it need to be scale * 2 ( the scaling now happen in the shader ) <- Done the * 2 is needed to map the -1 <-> 1 space to a 0 <-> 1 space 
        // Need to make a note about that

        auto shaderProgram = masterRenderer->getShader("text");

        for(const auto& renderList : currentRenderList)
        {
            auto texture = renderList.first;

            // Tex rendering
            shaderProgram->bind();

            shaderProgram->setUniformValue("projection", projection);
            shaderProgram->setUniformValue("model", model);
            shaderProgram->setUniformValue("scale", scale);

            shaderProgram->setUniformValue("time", static_cast<int>(0 % 314159));

            shaderProgram->setUniformValue("texture1", 0);

            //glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);

            // Todo combine all the call to the same texture into a single draw call using instanced rendering
            for(const auto& renderableTexture : renderList.second)
            {
                UiComponent *ui = renderableTexture.uiRef;

                auto mesh = renderableTexture.meshRef;

                if(not ui->isVisible() or not mesh)
                    continue;

                // Todo
                // view.translate(QVector3D(-1.0f + 2.0f * static_cast<UiSize>(ui->pos.x) / screenWidth, 1.0f + 2.0f * -static_cast<UiSize>(ui->pos.y) / screenHeight, -static_cast<UiSize>(ui->pos.z)));
                view = glm::mat4(1.0f);
                view = glm::translate(view, glm::vec3(-1.0f + 2.0f * static_cast<UiSize>(ui->pos.x) / screenWidth, 1.0f + 2.0f * -static_cast<UiSize>(ui->pos.y) / screenHeight, 0.0f));
                // glm::translate(view, glm::vec3(-0.5f , 0.5f, 1.0f));

                shaderProgram->setUniformValue("view", view);

                mesh->bind();
                glDrawElements(GL_TRIANGLES, mesh->modelInfo.nbIndices, GL_UNSIGNED_INT, 0);
            }
        }

        shaderProgram->release();
    }

    Mesh* SentenceSystem::getSentenceMesh(SentenceText& sentence, FontLoader *font)
    {
        // Todo add font name in mesh name
        auto meshName = "_sentence_" + sentence.text + "_" + std::to_string(sentence.scale);

        LOG_MILE("MeshBuilder", "Lookup for the sentence texture: " << meshName);

        const auto& it = meshes.find(meshName);

        if(it != meshes.end())
        {
            return it->second;
        }

        LOG_MILE("MeshBuilder", "Creating a new sentence texture mesh: " << meshName);

        auto mesh = new SentenceMesh();

        auto nbChara = sentence.text.length();
        
        mesh->modelInfo.nbVertices = 72 * nbChara;
        mesh->modelInfo.nbIndices = 6 * nbChara;

        if(mesh->modelInfo.vertices != nullptr)
            delete[] mesh->modelInfo.vertices;
        if(mesh->modelInfo.indices != nullptr)
            delete[] mesh->modelInfo.indices;

        mesh->modelInfo.vertices = new float [mesh->modelInfo.nbVertices];
        mesh->modelInfo.indices = new unsigned int [mesh->modelInfo.nbIndices];

        float currentX = 0.0f;
        float outO = 0.0f; //Outline Offset

        constant::ModelInfo letterModel; 

        for(size_t i = 0; i < nbChara; i++)
        {
            const auto letter = font->getChara(std::string(1, sentence.text.at(i)));

            outO = 0.0f;

            // Todo see how to add back the scale
            // auto w = letter->getWidth() * scale;
            // auto h = letter->getHeight() * scale;
            // auto o = letter->getOffset() * scale;

            auto w = letter->getWidth() * sentence.scale;
            auto h = letter->getHeight() * sentence.scale;
            auto o = letter->getOffset() * sentence.scale;

            if(sentence.outline1.w == 0.0f)
                outO += 1.0 * sentence.scale;

            if(sentence.outline2.w == 0.0f)
                outO += 1.0 * sentence.scale;

            letterModel = letter->getModelInfo();

            // Todo maybe also get the z to add it in the vertices
            // Coord
            mesh->modelInfo.vertices[i * 72 + 0]  = currentX - outO    ; mesh->modelInfo.vertices[i * 72 + 1]  =     -o + outO; mesh->modelInfo.vertices[i * 72 + 2]  = 0.0f;
            mesh->modelInfo.vertices[i * 72 + 18] = currentX + w + outO; mesh->modelInfo.vertices[i * 72 + 19] =     -o + outO; mesh->modelInfo.vertices[i * 72 + 20] = 0.0f;
            mesh->modelInfo.vertices[i * 72 + 36] = currentX - outO    ; mesh->modelInfo.vertices[i * 72 + 37] = -h - o - outO; mesh->modelInfo.vertices[i * 72 + 38] = 0.0f;
            mesh->modelInfo.vertices[i * 72 + 54] = currentX + w + outO; mesh->modelInfo.vertices[i * 72 + 55] = -h - o - outO; mesh->modelInfo.vertices[i * 72 + 56] = 0.0f;

            // Tex Coord
            mesh->modelInfo.vertices[i * 72 + 3]  = letterModel.vertices[3]  - outO / font->getAtlasWidth(); mesh->modelInfo.vertices[i * 72 + 4]  = letterModel.vertices[4]  - outO / font->getAtlasHeight();  
            mesh->modelInfo.vertices[i * 72 + 21] = letterModel.vertices[8]  + outO / font->getAtlasWidth(); mesh->modelInfo.vertices[i * 72 + 22] = letterModel.vertices[9]  - outO / font->getAtlasHeight();
            mesh->modelInfo.vertices[i * 72 + 39] = letterModel.vertices[13] - outO / font->getAtlasWidth(); mesh->modelInfo.vertices[i * 72 + 40] = letterModel.vertices[14] + outO / font->getAtlasHeight();
            mesh->modelInfo.vertices[i * 72 + 57] = letterModel.vertices[18] + outO / font->getAtlasWidth(); mesh->modelInfo.vertices[i * 72 + 58] = letterModel.vertices[19] + outO / font->getAtlasHeight();

            //Main Color
            mesh->modelInfo.vertices[i * 72 + 5]  = sentence.mainColor.x; mesh->modelInfo.vertices[i * 72 + 6]  = sentence.mainColor.y; mesh->modelInfo.vertices[i * 72 + 7]  = sentence.mainColor.z; mesh->modelInfo.vertices[i * 72 + 8]  = sentence.mainColor.w;
            mesh->modelInfo.vertices[i * 72 + 23] = sentence.mainColor.x; mesh->modelInfo.vertices[i * 72 + 24] = sentence.mainColor.y; mesh->modelInfo.vertices[i * 72 + 25] = sentence.mainColor.z; mesh->modelInfo.vertices[i * 72 + 26] = sentence.mainColor.w;
            mesh->modelInfo.vertices[i * 72 + 41] = sentence.mainColor.x; mesh->modelInfo.vertices[i * 72 + 42] = sentence.mainColor.y; mesh->modelInfo.vertices[i * 72 + 43] = sentence.mainColor.z; mesh->modelInfo.vertices[i * 72 + 44] = sentence.mainColor.w;
            mesh->modelInfo.vertices[i * 72 + 59] = sentence.mainColor.x; mesh->modelInfo.vertices[i * 72 + 60] = sentence.mainColor.y; mesh->modelInfo.vertices[i * 72 + 61] = sentence.mainColor.z; mesh->modelInfo.vertices[i * 72 + 62] = sentence.mainColor.w;

            //Outline 1
            mesh->modelInfo.vertices[i * 72 + 9]  = sentence.outline1.x; mesh->modelInfo.vertices[i * 72 + 10] = sentence.outline1.y; mesh->modelInfo.vertices[i * 72 + 11] = sentence.outline1.z; mesh->modelInfo.vertices[i * 72 + 12] = sentence.outline1.w;
            mesh->modelInfo.vertices[i * 72 + 27] = sentence.outline1.x; mesh->modelInfo.vertices[i * 72 + 28] = sentence.outline1.y; mesh->modelInfo.vertices[i * 72 + 29] = sentence.outline1.z; mesh->modelInfo.vertices[i * 72 + 30] = sentence.outline1.w;
            mesh->modelInfo.vertices[i * 72 + 45] = sentence.outline1.x; mesh->modelInfo.vertices[i * 72 + 46] = sentence.outline1.y; mesh->modelInfo.vertices[i * 72 + 47] = sentence.outline1.z; mesh->modelInfo.vertices[i * 72 + 48] = sentence.outline1.w;
            mesh->modelInfo.vertices[i * 72 + 63] = sentence.outline1.x; mesh->modelInfo.vertices[i * 72 + 64] = sentence.outline1.y; mesh->modelInfo.vertices[i * 72 + 65] = sentence.outline1.z; mesh->modelInfo.vertices[i * 72 + 66] = sentence.outline1.w;

            //Outline 2
            mesh->modelInfo.vertices[i * 72 + 13] = sentence.outline2.x; mesh->modelInfo.vertices[i * 72 + 14] = sentence.outline2.y; mesh->modelInfo.vertices[i * 72 + 15] = sentence.outline2.z; mesh->modelInfo.vertices[i * 72 + 16] = sentence.outline2.w;
            mesh->modelInfo.vertices[i * 72 + 31] = sentence.outline2.x; mesh->modelInfo.vertices[i * 72 + 32] = sentence.outline2.y; mesh->modelInfo.vertices[i * 72 + 33] = sentence.outline2.z; mesh->modelInfo.vertices[i * 72 + 34] = sentence.outline2.w;
            mesh->modelInfo.vertices[i * 72 + 49] = sentence.outline2.x; mesh->modelInfo.vertices[i * 72 + 50] = sentence.outline2.y; mesh->modelInfo.vertices[i * 72 + 51] = sentence.outline2.z; mesh->modelInfo.vertices[i * 72 + 52] = sentence.outline2.w;
            mesh->modelInfo.vertices[i * 72 + 67] = sentence.outline2.x; mesh->modelInfo.vertices[i * 72 + 68] = sentence.outline2.y; mesh->modelInfo.vertices[i * 72 + 69] = sentence.outline2.z; mesh->modelInfo.vertices[i * 72 + 70] = sentence.outline2.w;

            //Effect
            mesh->modelInfo.vertices[i * 72 + 17] = (float)sentence.effect;
            mesh->modelInfo.vertices[i * 72 + 35] = (float)sentence.effect;
            mesh->modelInfo.vertices[i * 72 + 53] = (float)sentence.effect;
            mesh->modelInfo.vertices[i * 72 + 71] = (float)sentence.effect;

            mesh->modelInfo.indices[i * 6 + 0] = 4 * i + 0; mesh->modelInfo.indices[i * 6 + 1] = 4 * i + 1; mesh->modelInfo.indices[i * 6 + 2] = 4 * i + 2;
            mesh->modelInfo.indices[i * 6 + 3] = 4 * i + 1; mesh->modelInfo.indices[i * 6 + 4] = 4 * i + 2; mesh->modelInfo.indices[i * 6 + 5] = 4 * i + 3;

            currentX += w + 1;

            if(h + o > sentence.textHeight)
                sentence.textHeight = h + o;

        }

        sentence.textWidth = currentX;

        meshes.emplace(meshName, mesh);

        return mesh;
    }

    /** Helper that create an entity with an Ui component and a Texture component */
    CompList<UiComponent, SentenceText> makeSentence(EntitySystem *ecs, float x, float y, const SentenceText& text)
    {
        auto entity = ecs->createEntity();

        auto ui = ecs->attach<UiComponent>(entity);

        auto sentence = ecs->attach<SentenceText>(entity, text);

        ui->setX(x);
        ui->setY(y);

        // Todo fix this in ui component so it does work, right now if the entity is created during runtime the link is not made correctly
        ui->setWidth(sentence->textWidth);
        ui->setHeight(sentence->textHeight);

        return CompList<UiComponent, SentenceText>(entity, ui, sentence);
    }
}