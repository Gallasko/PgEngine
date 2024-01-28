#include "texture.h"

#include "logger.h"
#include "serialization.h"

#include "Renderer/renderer.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "Helpers/openglobject.h"

namespace pg
{
    namespace
    {
        static constexpr char const * DOM = "Texture";

        static constexpr char const * AtlasTextureDelimiter = "._";

        std::string getMainAtlasTexture(const std::string& textureName)
        {
            auto it = textureName.find(AtlasTextureDelimiter);
            
            if (it != std::string::npos)
            {
                return textureName.substr(0, it);
            }

            return textureName;
        }
    }


    template <>
    void serialize(Archive& archive, const Texture2DComponent& value)
    {
        LOG_THIS(DOM);

        archive.startSerialization(Texture2DComponent::getType());

        serialize(archive, "textureName", value.textureName);
    
        archive.endSerialization();
    }

    template <>
    Texture2DComponent deserialize(const UnserializedObject& serializedString)
    {
        LOG_THIS(DOM);

        std::string type = "";

        if(serializedString.isNull())
            LOG_ERROR(DOM, "Element is null");
        else
        {
            LOG_INFO(DOM, "Deserializing an Texture2DComponent");

            auto textureName = deserialize<std::string>(serializedString["textureName"]);

            return Texture2DComponent{textureName};
        }

        return Texture2DComponent{""};
    }

    void TextureMesh::generateMesh()
    {
        LOG_THIS_MEMBER("Texture Mesh");

        openGLMesh.initialize();

        openGLMesh.VAO->bind();

        // position attribute
        openGLMesh.VBO->bind();
        openGLMesh.VBO->setUsagePattern(OpenGLBuffer::StreamDraw);
        openGLMesh.VBO->allocate(modelInfo.vertices, modelInfo.nbVertices * sizeof(float));

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

        // texture coord attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        openGLMesh.EBO->bind();
        openGLMesh.EBO->setUsagePattern(OpenGLBuffer::StreamDraw);
        openGLMesh.EBO->allocate(modelInfo.indices, modelInfo.nbIndices * sizeof(unsigned int));

        openGLMesh.VAO->release();

        initialized = true;
    }

    void Texture2DComponentSystem::TextureBuffer::SimpleSquareMesh::generateMesh()
    {
        LOG_THIS_MEMBER("Shape 2D Mesh");

        openGLMesh.initialize();

        openGLMesh.VAO->bind();

        openGLMesh.VBO->bind();
        openGLMesh.VBO->setUsagePattern(OpenGLBuffer::StaticDraw);
        openGLMesh.VBO->allocate(modelInfo.vertices, modelInfo.nbVertices * sizeof(float));

        // Position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        instanceVBO = new OpenGLBuffer(OpenGLBuffer::VertexBuffer);
        instanceVBO->setUsagePattern(OpenGLBuffer::DynamicDraw);
        instanceVBO->create();

        instanceVBO->bind();

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribDivisor(2, 1); // tell OpenGL this is an instanced vertex attribute.
        glVertexAttribDivisor(3, 1); // tell OpenGL this is an instanced vertex attribute.
        glVertexAttribDivisor(4, 1); // tell OpenGL this is an instanced vertex attribute.

        openGLMesh.EBO->bind();
        openGLMesh.EBO->setUsagePattern(OpenGLBuffer::StaticDraw);
        openGLMesh.EBO->allocate(modelInfo.indices, modelInfo.nbIndices * sizeof(unsigned int));

        openGLMesh.VAO->release();

        initialized = true;
    }

    Texture2DComponentSystem::TextureBuffer::SimpleSquareMesh::~SimpleSquareMesh()
    { 
        LOG_THIS_MEMBER("Texture Component System");

        if (instanceVBO)
            delete instanceVBO;
    }

    void Texture2DComponentSystem::init()
    {
        auto group = registerGroup<UiComponent, Texture2DComponent>();

        group->addOnGroup([](EntityRef entity) {
            LOG_INFO("Texture Component System", "Add entity " << entity->id << " to ui - tex group !");

            auto ui = entity->get<UiComponent>();
            auto tex = entity->get<Texture2DComponent>();

            auto tName = tex->textureName;

            auto sys = entity->world()->getSystem<Texture2DComponentSystem>();

            auto mesh = sys->getTextureMesh(ui->width, ui->height, tName);

            auto rTex = RenderableTexture{entity->id, ui, mesh};

            auto textureId = sys->masterRenderer->getTexture(tName);

            sys->tempRenderList[textureId].push_back(rTex);

            sys->changed = true;
        });
    }

    void Texture2DComponentSystem::onEvent(const UiComponentChangeEvent& event)
    {
        auto entity = ecsRef->getEntity(event.id);

        if (not entity or not entity->has<Texture2DComponent>() or not entity->has<UiComponent>())
            return;

        auto ui = entity->get<UiComponent>();

        auto tex = entity->get<Texture2DComponent>();

        auto tName = tex->textureName;

        auto sys = entity->world()->getSystem<Texture2DComponentSystem>();

        auto mesh = sys->getTextureMesh(ui->width, ui->height, tName);

        auto rTex = RenderableTexture{event.id, ui, mesh};

        LOG_MILE("Texture Component System", "Modification of id: " << entity->id << " texture");

        auto textureId = sys->masterRenderer->getTexture(tName);

        std::lock_guard<std::mutex> lock (sys->modificationMutex);

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

    void Texture2DComponentSystem::render()
    {
        LOG_THIS(DOM);
    
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

        auto shaderProgram = masterRenderer->getShader("default");

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

                if(not ui or not ui->isVisible() or not mesh)
                    continue;

                // Todo
                // view.translate(QVector3D(-1.0f + 2.0f * static_cast<UiSize>(ui->pos.x) / screenWidth, 1.0f + 2.0f * -static_cast<UiSize>(ui->pos.y) / screenHeight, -static_cast<UiSize>(ui->pos.z)));
                view = glm::mat4(1.0f);
                view = glm::translate(view, glm::vec3(-1.0f + 2.0f * ui->pos.x / screenWidth, 1.0f + 2.0f * -ui->pos.y / screenHeight, 0.0f));
                // glm::translate(view, glm::vec3(-0.5f , 0.5f, 1.0f));

                shaderProgram->setUniformValue("view", view);

                mesh->bind();
                glDrawElements(GL_TRIANGLES, mesh->modelInfo.nbIndices, GL_UNSIGNED_INT, 0);
            }
        }

        shaderProgram->release();
    }

    Mesh* Texture2DComponentSystem::getTextureMesh(float width, float height, const std::string& name)
    {
        LOG_THIS_MEMBER(DOM);

        auto meshName = "_texture_" + name + "_" + std::to_string(width) + "_" + std::to_string(height);

        const auto& it = meshes.find(meshName);

        if(it == meshes.end())
        {
            LOG_MILE(DOM, "Creating a new texture mesh: " << meshName);

            auto mesh = new TextureMesh();

            mesh->modelInfo.vertices[0] =  0.0f;  mesh->modelInfo.vertices[1] =    0.0f;   mesh->modelInfo.vertices[2] =  0.0f;
            mesh->modelInfo.vertices[5] =  width; mesh->modelInfo.vertices[6] =    0.0f;   mesh->modelInfo.vertices[7] =  0.0f;
            mesh->modelInfo.vertices[10] = 0.0f;  mesh->modelInfo.vertices[11] =  -height; mesh->modelInfo.vertices[12] = 0.0f;
            mesh->modelInfo.vertices[15] = width; mesh->modelInfo.vertices[16] =  -height; mesh->modelInfo.vertices[17] = 0.0f;

            meshes.emplace(meshName, mesh);

            return mesh;
        }
        else
        {
            // Todo increment the number of time this mesh is used
            // m_meshes[meshName].count++;
        }

        return it->second;
    }

    void Texture2DComponentSystem::addElement(const CompRef<UiComponent>& ui, const CompRef<Texture2DComponent>& obj)
    {
        LOG_THIS_MEMBER(DOM);

        LOG_INFO(DOM, "Add element " << ui.entityId << " to buffer !");

        auto name = obj->textureName;

        auto mainTexName = getMainAtlasTexture(name);

        auto texBuffer = getTexBuffer(mainTexName);

        std::lock_guard<std::mutex> lock(texBuffer->renderMutex);

        texBuffer->idToIndexMap[ui.entityId] = texBuffer->elementIndex;

        auto currentIndex = texBuffer->elementIndex++;

        auto visible = ui->isVisible();
        
        texBuffer->bufferData[currentIndex * texBuffer->nbAttributes + 0] = ui->pos.x;
        texBuffer->bufferData[currentIndex * texBuffer->nbAttributes + 1] = ui->pos.y;
        texBuffer->bufferData[currentIndex * texBuffer->nbAttributes + 2] = ui->pos.z;

        texBuffer->bufferData[currentIndex * texBuffer->nbAttributes + 3] = ui->width;
        texBuffer->bufferData[currentIndex * texBuffer->nbAttributes + 4] = ui->height;

        if (obj->additionnalParameters.size() == (texBuffer->nbAttributes - 5))
        {
            for (size_t i = 0; i < texBuffer->nbAttributes - 5; i++)
            {
                texBuffer->bufferData[currentIndex * texBuffer->nbAttributes + 5 + i] = obj->additionnalParameters.at(i);
            }
        }
        else
        {
            LOG_ERROR(DOM, "Failed to add additional parameters, setting all of them to 0");

            for (size_t i = 0; i < texBuffer->nbAttributes - 5; i++)
            {
                texBuffer->bufferData[currentIndex * texBuffer->nbAttributes + 5 + i] = 0;
            }
        }

        if (visible)
        {
            auto index = texBuffer->visibleElements++;

            if (texBuffer->visibleElements != texBuffer->elementIndex)
            {
                texBuffer->swapIndex(currentIndex, index);
            }
        }  

        if(currentIndex + 1 >= texBuffer->currentSize)
        {
            float *temp = new float[2 * texBuffer->currentSize * texBuffer->nbAttributes];

            memcpy(temp, texBuffer->bufferData, texBuffer->currentSize * texBuffer->nbAttributes * sizeof(float));

            texBuffer->currentSize *= 2;

            delete texBuffer->bufferData;

            texBuffer->bufferData = temp;

            texBuffer->sizeChanged = true;
        }

        changed = true;
    }

    void Texture2DComponentSystem::removeElement(_unique_id id)
    {
        LOG_THIS_MEMBER(DOM);

        auto buffer = getTexBuffer(id);

        if (buffer == nullptr)
        {
            LOG_ERROR(DOM, "No buffer found with id " << id);
            return;
        }

        if (buffer->elementIndex == 0)
        {
            LOG_ERROR(DOM, "This should never be the case");
            return;
        }
        else if (buffer->elementIndex == 1)
        {
            changed = true;
            buffer->elementIndex = 0;
            return;
        }

        auto lastElement = --buffer->elementIndex;

        auto prev = buffer->idToIndexMap[id];

        bool needToSwap = false;

        // If the element to be deleted was visible we remove it from the visible list
        if (prev < buffer->visibleElements)
        {
            buffer->visibleElements--;

            // If prev is still less than visible elements then it wasn't the last visible element in the list so we need to swap a visible element at is place
            needToSwap = prev < buffer->visibleElements and buffer->elementIndex != buffer->visibleElements;
        }

        buffer->idToIndexMap[id] = buffer->idToIndexMap[lastElement];

        std::lock_guard<std::mutex> lock (modificationMutex);

        // Swap the last element at the removed place

        for (size_t i = 0; i < buffer->nbAttributes; i++)
        {
            buffer->bufferData[prev * buffer->nbAttributes + i] = buffer->bufferData[lastElement * buffer->nbAttributes + i];
        }

        // The remove item was a visible one so we need to swap a visible one at this place
        if (needToSwap)
        {
            // We swap the last visible element then
            buffer->swapIndex(prev, buffer->visibleElements.load());
        }

        changed = true;
    }

    void Texture2DComponentSystem::TextureBuffer::swapIndex(size_t origin, size_t destination)
    {
        LOG_THIS_MEMBER(DOM);

        auto originId = std::find_if(
            idToIndexMap.begin(),
            idToIndexMap.end(),
            [origin](const auto& mo) {return mo.second == origin; });

        auto destinationId = std::find_if(
            idToIndexMap.begin(),
            idToIndexMap.end(),
            [destination](const auto& mo) {return mo.second == destination; });

        if (originId == idToIndexMap.end() or destinationId == idToIndexMap.end())
        {
            LOG_ERROR(DOM, "Error swapping index !");
            return;
        }

        LOG_INFO(DOM, "Swapping index of " << originId->first << " [" << idToIndexMap[originId->first] <<
                       "] to index of " << destinationId->first << " [" << idToIndexMap[destinationId->first] << "]");

        idToIndexMap[originId->first] = destination;
        idToIndexMap[destinationId->first] = origin;

        float temp[nbAttributes];

        for (size_t i = 0; i < nbAttributes; i++)
        {
            temp[i] = bufferData[origin * nbAttributes + i];
        }

        for (size_t i = 0; i < nbAttributes; i++)
        {
            bufferData[origin * nbAttributes + i] = bufferData[destination * nbAttributes + i];
        }

        for (size_t i = 0; i < nbAttributes; i++)
        {
            bufferData[destination * nbAttributes + i] = temp[i];
        }

        LOG_INFO(DOM, "Done Swapping data and index of " << originId->first << " [" << idToIndexMap[originId->first] <<
                       "] to index of " << destinationId->first << " [" << idToIndexMap[destinationId->first] << "]");
    }

    std::shared_ptr<Texture2DComponentSystem::TextureBuffer> Texture2DComponentSystem::getTexBuffer(const std::string& mainTexName)
    {
        LOG_THIS_MEMBER(DOM);

        auto it = textureBuffers.find(mainTexName);

        if (it != textureBuffers.end())
        {
            return it->second;
        }

        return nullptr;
    }

    std::shared_ptr<Texture2DComponentSystem::TextureBuffer> Texture2DComponentSystem::getTexBuffer(_unique_id id)
    {
        LOG_THIS_MEMBER(DOM);

        for (const auto& buffer : textureBuffers)
        {
            if (buffer.second->idToIndexMap.find(id) != buffer.second->idToIndexMap.end())
            {
                return buffer.second;
            }
        }

        return nullptr;
    }

    CompList<UiComponent, Texture2DComponent> makeUiTexture(EntitySystem *ecs, float width, float height, const std::string& name)
    {
        auto entity = ecs->createEntity();

        auto ui = ecs->attach<UiComponent>(entity);

        ui->setWidth(width);
        ui->setHeight(height);

        auto tex = ecs->attach<Texture2DComponent>(entity, name);

        return CompList<UiComponent, Texture2DComponent>(entity, ui, tex);
    }
}