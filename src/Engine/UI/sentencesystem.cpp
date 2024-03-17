#include "Helpers/openglobject.h"

#include "sentencesystem.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

namespace pg
{
    namespace
    {
        constexpr const char * const DOM = "Sentence System";
    }

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

    void SentenceText::onDeletion(EntityRef entity) 
    {
        auto sys = ecsRef->getSystem<SentenceSystem>();

        for (const auto& letter : letters)
        {
            sys->removeElement(letter.id);
        }
    }

    void SentenceSystem::SimpleSquareMesh::generateMesh()
    {
        LOG_THIS_MEMBER("Shape 2D Mesh");

        openGLMesh.initialize();

        openGLMesh.VAO->bind();

        openGLMesh.VBO->bind();
        openGLMesh.VBO->setUsagePattern(OpenGLBuffer::StaticDraw);
        openGLMesh.VBO->allocate(modelInfo.vertices, modelInfo.nbVertices * sizeof(float));

        // Position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        instanceVBO = new OpenGLBuffer(OpenGLBuffer::VertexBuffer);
        instanceVBO->setUsagePattern(OpenGLBuffer::DynamicDraw);
        instanceVBO->create();

        instanceVBO->bind();

        // World pos (x, y, z) 3 floats
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 22 * sizeof(float), (void*)0);

        // Texture coord (xmin, ymin, xmax, ymax) 4 floats
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 22 * sizeof(float), (void*)(3 * sizeof(float)));

        // Size (w, h) 2 floats
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 22 * sizeof(float), (void*)(7 * sizeof(float)));

        // Color 1 (r, g, b, a) 4 floats
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 22 * sizeof(float), (void*)(9 * sizeof(float)));

        // Color 2 (r, g, b, a) 4 floats
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 22 * sizeof(float), (void*)(13 * sizeof(float)));

        // Color 3 (r, g, b, a) 4 floats
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 22 * sizeof(float), (void*)(17 * sizeof(float)));

        // Effect 1 float
        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, 22 * sizeof(float), (void*)(21 * sizeof(float)));


        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribDivisor(1, 1); // tell OpenGL this is an instanced vertex attribute.
        glVertexAttribDivisor(2, 1); // tell OpenGL this is an instanced vertex attribute.
        glVertexAttribDivisor(3, 1); // tell OpenGL this is an instanced vertex attribute.
        glVertexAttribDivisor(4, 1); // tell OpenGL this is an instanced vertex attribute.
        glVertexAttribDivisor(5, 1); // tell OpenGL this is an instanced vertex attribute.
        glVertexAttribDivisor(6, 1); // tell OpenGL this is an instanced vertex attribute.
        glVertexAttribDivisor(7, 1); // tell OpenGL this is an instanced vertex attribute.

        openGLMesh.EBO->bind();
        openGLMesh.EBO->setUsagePattern(OpenGLBuffer::StaticDraw);
        openGLMesh.EBO->allocate(modelInfo.indices, modelInfo.nbIndices * sizeof(unsigned int));

        openGLMesh.VAO->release();

        initialized = true;
    }

    SentenceSystem::SimpleSquareMesh::~SimpleSquareMesh()
    { 
        LOG_THIS_MEMBER("Shape 2D Mesh");

        if (instanceVBO)
            delete instanceVBO;
    }

    void SentenceMesh::generateMesh()
    {
        LOG_THIS_MEMBER("Sentence Mesh");

        openGLMesh.initialize();

        openGLMesh.VAO->bind();

        // position attribute
        openGLMesh.VBO->bind();
        openGLMesh.VBO->setUsagePattern(OpenGLBuffer::StaticDraw);
        openGLMesh.VBO->allocate(modelInfo.vertices, modelInfo.nbVertices * sizeof(float));

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

        openGLMesh.EBO->bind();
        openGLMesh.EBO->setUsagePattern(OpenGLBuffer::StaticDraw);
        openGLMesh.EBO->allocate(modelInfo.indices, modelInfo.nbIndices * sizeof(unsigned int));

        openGLMesh.VAO->release();

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

            sys->generateLetters(*sentence, sys->font);

            sys->addElement(ui, sentence);
        });
    }

    void SentenceSystem::onEvent(const OnTextChanged& event)
    {
        auto entity = ecsRef->getEntity(event.entityId);

        auto ui = entity->get<UiComponent>();
        auto sentence = entity->get<SentenceText>();

        for (const auto& letter : sentence->letters)
        {
            removeElement(letter.id);
        }

        sentence->letters.clear();

        sentence->text = event.newText;

        generateLetters(*sentence, font);

        addElement(ui, sentence);
    }

    void SentenceSystem::onEvent(const UiComponentChangeEvent& event)
    {
        auto entity = ecsRef->getEntity(event.id);

        if(not entity or not entity->has<SentenceText>() or not entity->has<UiComponent>())
            return;

        size_t index;

        auto ui = entity->get<UiComponent>();
        auto obj = entity->get<SentenceText>();

        float x = ui->pos.x;
        float y = ui->pos.y;
        float z = ui->pos.z;

        // Changing width and height of a text component should have no effect as the width and height are dependant on the text itself

        bool visible = ui->isVisible();

        float currentX = 0.0f;
        float outO = 0.0f;    //Outline Offset

        {
            std::lock_guard<std::mutex> lock(modificationMutex);

            LOG_INFO(DOM, "-------------------- Modification of sentence pos of sentence id: " << event.id << " --------------------");

            for (const auto& letter : obj->letters)
            {
                auto it = idToIndexMap.find(letter.id);
    
                if (it == idToIndexMap.end())
                    return;

                index = it->second;

                auto fontInfo = letter.font;

                auto w = fontInfo->getWidth()  * obj->scale;
                auto h = fontInfo->getHeight() * obj->scale;
                auto o = fontInfo->getOffset() * obj->scale;

                outO = 0.0f;

                if(obj->outline1.w == 0.0f)
                    outO += 1.0 * obj->scale;

                if(obj->outline2.w == 0.0f)
                    outO += 1.0 * obj->scale;

                LOG_INFO(DOM, "Putting: " << fontInfo->getName() << " at " << x + currentX + outO << ", " << y + o + outO << " with size = " << w - outO << ", " << w - outO);

                 // World pos (x, y, z) 3 floats
                bufferData[index * nbAttributes + 0] = x + currentX + outO;
                bufferData[index * nbAttributes + 1] = y + o + outO;
                bufferData[index * nbAttributes + 2] = z;
 
                currentX += w - (2.0 * outO) + 1;

                // Swap the element toward visible or not if it is not in the same state as before
                if (index < visibleElements and not visible)
                {
                    auto lastVisible = --visibleElements;

                    if (index != lastVisible)
                        swapIndex(index, lastVisible);
                }
                else if (index >= visibleElements and visible)
                {
                    auto lastVisible = visibleElements++;

                    if (index != lastVisible)
                        swapIndex(index, lastVisible);
                }    
            }

            changed = true;
        }
    }

    void SentenceSystem::render()
    {
        static size_t nbElements = 0;

        if (changed)
        {
            std::lock_guard<std::mutex> lock(modificationMutex);

            if (not squareMeshInitialized)
            {
                basicSquareMesh.generateMesh();

                squareMeshInitialized = true;
            }

            basicSquareMesh.openGLMesh.VAO->bind();

            auto size = visibleElements.load();

            basicSquareMesh.bind();

            basicSquareMesh.instanceVBO->allocate(bufferData, size * nbAttributes * sizeof(float));

            nbElements = elementIndex.load();

            changed = false;
        }

        if (not squareMeshInitialized or nbElements <= 0)
            return;

        auto rTable = masterRenderer->getParameter();
        const int screenWidth = rTable["ScreenWidth"];
        const int screenHeight = rTable["ScreenHeight"];

        glm::mat4 projection = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 scale = glm::mat4(1.0f);

        scale = glm::scale(scale, glm::vec3(2.0f / screenWidth, 2.0f / screenHeight, 1.0f));

        // TODO why does it need to be scale * 2 ( the scaling now happen in the shader ) <- Done the * 2 is needed to map the -1 <-> 1 space to a 0 <-> 1 space 
        // Need to make a note about that

        auto shaderProgram = masterRenderer->getShader("text");

        auto texture = masterRenderer->getTexture("font");

        shaderProgram->bind();

        glBindTexture(GL_TEXTURE_2D, texture);

        shaderProgram->setUniformValue("sWidth", static_cast<float>(screenWidth));
        shaderProgram->setUniformValue("sHeight", static_cast<float>(screenHeight));

        shaderProgram->setUniformValue("tWidth", static_cast<float>(font->getAtlasWidth()));
        shaderProgram->setUniformValue("tHeight", static_cast<float>(font->getAtlasHeight()));

        shaderProgram->setUniformValue("projection", projection);
        shaderProgram->setUniformValue("model", model);
        shaderProgram->setUniformValue("scale", scale);
        shaderProgram->setUniformValue("view", view);

        shaderProgram->setUniformValue("time", static_cast<int>(0 % 314159));

        shaderProgram->setUniformValue("texture1", 0);

        basicSquareMesh.bind();
        glDrawElementsInstanced(GL_TRIANGLES, basicSquareMesh.modelInfo.nbIndices, GL_UNSIGNED_INT, 0, nbElements);

        shaderProgram->release();
    }

    void SentenceSystem::addElement(const CompRef<UiComponent>& ui, const CompRef<SentenceText>& obj)
    {
        LOG_INFO(DOM, "Add element " << ui.entityId << " to buffer !");

        {
            std::lock_guard<std::mutex> lock (modificationMutex);

            while (obj->letters.size() + elementIndex >= currentSize)
            {
                increaseSize();
            }

            float currentX = 0.0f;
            float outO = 0.0f;    //Outline Offset

            float x = ui->pos.x;
            float y = ui->pos.y;
            float z = ui->pos.z;

            auto visible = ui->isVisible();

            for (const auto& letter : obj->letters)
            {
                idToIndexMap[letter.id] = elementIndex;

                auto currentIndex = elementIndex++;

                auto fontInfo = letter.font;

                const auto& textureLimit = fontInfo->getTextureLimit();

                auto w = fontInfo->getWidth()  * obj->scale;
                auto h = fontInfo->getHeight() * obj->scale;
                auto o = fontInfo->getOffset() * obj->scale;

                outO = 0.0f;

                if(obj->outline1.w == 0.0f)
                    outO += 1.0 * obj->scale;

                if(obj->outline2.w == 0.0f)
                    outO += 1.0 * obj->scale;

                LOG_INFO(DOM, "Putting: " << fontInfo->getName() << " at " << x + currentX + outO << ", " << y + o + outO << " with size = " << w - outO << ", " << w - outO);

                 // World pos (x, y, z) 3 floats
                bufferData[currentIndex * nbAttributes + 0] = x + currentX + outO;
                bufferData[currentIndex * nbAttributes + 1] = y + o + outO;
                bufferData[currentIndex * nbAttributes + 2] = z;

                // Texture coord (xmin, ymin, xmax, ymax) 4 floats
                bufferData[currentIndex * nbAttributes + 3] = textureLimit.x;
                bufferData[currentIndex * nbAttributes + 4] = textureLimit.y;
                bufferData[currentIndex * nbAttributes + 5] = textureLimit.z;
                bufferData[currentIndex * nbAttributes + 6] = textureLimit.w;

                // Size (w, h) 2 floats
                bufferData[currentIndex * nbAttributes + 7] = w - outO;
                bufferData[currentIndex * nbAttributes + 8] = h - outO;

                // Color 1 (r, g, b, a) 4 floats
                bufferData[currentIndex * nbAttributes + 9]  = obj->mainColor.x;
                bufferData[currentIndex * nbAttributes + 10] = obj->mainColor.y;
                bufferData[currentIndex * nbAttributes + 11] = obj->mainColor.z;
                bufferData[currentIndex * nbAttributes + 12] = obj->mainColor.w;

                // Color 2 (r, g, b, a) 4 floats
                bufferData[currentIndex * nbAttributes + 13] = obj->outline1.x;
                bufferData[currentIndex * nbAttributes + 14] = obj->outline1.y;
                bufferData[currentIndex * nbAttributes + 15] = obj->outline1.z;
                bufferData[currentIndex * nbAttributes + 16] = obj->outline1.w;

                // Color 3 (r, g, b, a) 4 floats
                bufferData[currentIndex * nbAttributes + 17] = obj->outline2.x;
                bufferData[currentIndex * nbAttributes + 18] = obj->outline2.y;
                bufferData[currentIndex * nbAttributes + 19] = obj->outline2.z;
                bufferData[currentIndex * nbAttributes + 20] = obj->outline2.w;

                // Effect 1 float
                bufferData[currentIndex * nbAttributes + 21] = static_cast<float>(obj->effect);

                currentX += w - (2.0 * outO) + 1;

                if (visible)
                {
                    auto index = visibleElements++;

                    if (visibleElements != elementIndex)
                    {
                        swapIndex(currentIndex, index);
                    }
                }
            }

            changed = true;
        }
    }

    void SentenceSystem::generateLetters(SentenceText& sentence, FontLoader *font)
    {
        LOG_MILE("MeshBuilder", "Creating a new sentence texture mesh: " << sentence.text);

        if(not font or font->isEmpty())
        {
            LOG_ERROR("MeshBuilder", "No font loaded");
            return;
        }

        auto nbChara = sentence.text.length();
        
        float currentX = 0.0f;
        float outO = 0.0f;    //Outline Offset

        for(size_t i = 0; i < nbChara; i++)
        {
            // Todo fix this for emscripten
            // const auto letter = font->getChara(sentence.text[i]);
            const auto letter = font->getChara(std::string(1, sentence.text.at(i)));

            auto id = nextLetterId++;

            sentence.letters.emplace_back(id, letter);

            outO = 0.0f;

            auto w = letter->getWidth() * sentence.scale;
            auto h = letter->getHeight() * sentence.scale;
            auto o = letter->getOffset() * sentence.scale;

            if(sentence.outline1.w == 0.0f)
                outO += 1.0 * sentence.scale;

            if(sentence.outline2.w == 0.0f)
                outO += 1.0 * sentence.scale;

            currentX += w + 1;

            if(h + o > sentence.textHeight)
                sentence.textHeight = h + o;

        }

        sentence.textWidth = currentX;
    }

    /** Helper that create an entity with an Ui component and a Texture component */
    CompList<UiComponent, SentenceText> makeSentence(EntitySystem *ecs, float x, float y, const SentenceText& text)
    {
        auto entity = ecs->createEntity();

        auto ui = ecs->attach<UiComponent>(entity);

        auto sentence = ecs->attach<SentenceText>(entity, text);

        ui->setX(x);
        ui->setY(y);

        ui->setWidth(&sentence->textWidth);
        ui->setHeight(&sentence->textHeight);

        return CompList<UiComponent, SentenceText>(entity, ui, sentence);
    }
}