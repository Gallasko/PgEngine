#include "texture.h"

#include "logger.h"
#include "serialization.h"

#include "Renderer/renderer.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "Helpers/openglobject.h"

namespace pg
{
    static constexpr char const * DOM = "Texture";

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

    void Texture2DComponentSystem::init()
    {
        auto group = registerGroup<UiComponent, Texture2DComponent>();

        group->addOnGroup([](EntityRef entity) {
            LOG_MILE("Texture Component System", "Add entity " << entity->id << " to ui - tex group !");

            auto ui = entity->get<UiComponent>();
            auto tex = entity->get<Texture2DComponent>();

            auto tName = tex->textureName;

            auto sys = entity->world()->getSystem<Texture2DComponentSystem>();

            auto mesh = sys->getTextureMesh(ui->width, ui->height, tName);

            auto rTex = RenderableTexture{entity->id, ui, mesh};

            auto textureId = sys->masterRenderer->getTexture(tName);

            std::lock_guard<std::mutex> lock (sys->modificationMutex);

            sys->tempRenderList[textureId].push_back(rTex);

            sys->changed = true;
        });
    }

    void Texture2DComponentSystem::onEvent(const UiComponentChangeEvent& event)
    {
        auto entity = ecsRef->getEntity(event.id);

        if(not entity or not entity->has<Texture2DComponent>() or not entity->has<UiComponent>())
            return;

        auto ui = entity->get<UiComponent>();

        // Todo check if the entity has a sentence text before trying to modify it
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

    void Texture2DComponentSystem::onEvent(const TextureChangeEvent& event)
    {
        LOG_MILE(DOM, "On texture change event: " << event.id << " texture to change : " << event.oldTextureName << " with new texture : " << event.newTextureName);
        auto entity = ecsRef->getEntity(event.id);

        if (event.oldTextureName == event.newTextureName or not entity or not entity->has<Texture2DComponent>() or not entity->has<UiComponent>())
            return;

        auto ui = entity->get<UiComponent>();

        auto sys = entity->world()->getSystem<Texture2DComponentSystem>();

        auto mesh = sys->getTextureMesh(ui->width, ui->height, event.newTextureName);

        auto rTex = RenderableTexture{event.id, ui, mesh};

        LOG_MILE("Texture Component System", "Modification of id: " << entity->id << " texture");

        auto oldTextureId = sys->masterRenderer->getTexture(event.oldTextureName);
        auto textureId = sys->masterRenderer->getTexture(event.newTextureName);

        std::lock_guard<std::mutex> lock (sys->modificationMutex);

        auto first = sys->tempRenderList[oldTextureId].begin();
        auto last = sys->tempRenderList[oldTextureId].end();

        while (first != last)
        {
            if (first->entityId == event.id)
            {
                sys->tempRenderList[oldTextureId].erase(first);
                break;
            }

            ++first;
        }

        sys->tempRenderList[textureId].push_back(rTex);

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
                view = glm::translate(view, glm::vec3(-1.0f + 2.0f * ui->pos.x / screenWidth, 1.0f + 2.0f * -ui->pos.y / screenHeight, ui->pos.z));
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
        LOG_THIS_MEMBER("MeshBuilder");

        auto meshName = "_texture_" + name + "_" + std::to_string(width) + "_" + std::to_string(height);

        const auto& it = meshes.find(meshName);

        if(it == meshes.end())
        {
            LOG_MILE("MeshBuilder", "Creating a new texture mesh: " << meshName);

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