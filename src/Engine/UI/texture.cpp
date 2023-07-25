#include "texture.h"

#include "logger.h"
#include "serialization.h"

#include "Renderer/renderer.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "openglobject.h"

namespace pg
{
    static constexpr char const * DOM = "Texture";

    template <>
    void serialize(Archive& archive, const TextureComponent& value)
    {
        LOG_THIS(DOM);

        archive.startSerialization("TextureComponent");

        serialize(archive, "textureName", value.textureName);
    
        archive.endSerialization();
    }

    template <>
    TextureComponent deserialize(const UnserializedObject& serializedString)
    {
        LOG_THIS(DOM);

        std::string type = "";

        if(serializedString.isNull())
            LOG_ERROR(DOM, "Element is null");
        else
        {
            LOG_INFO(DOM, "Deserializing an TextureComponent");

            auto textureName = deserialize<std::string>(serializedString["textureName"]);

            return TextureComponent{textureName};
        }

        return TextureComponent{""};
    }

    void TextureMesh::generateMesh()
    {
        LOG_THIS_MEMBER("Texture Mesh");

        OpenGLMesh.initialize();

        OpenGLMesh.VAO->bind();

        // position attribute
        OpenGLMesh.VBO->bind();
        OpenGLMesh.VBO->setUsagePattern(OpenGLBuffer::StreamDraw);
        OpenGLMesh.VBO->allocate(modelInfo.vertices, modelInfo.nbVertices * sizeof(float));

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

        // texture coord attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        OpenGLMesh.EBO->bind();
        OpenGLMesh.EBO->setUsagePattern(OpenGLBuffer::StreamDraw);
        OpenGLMesh.EBO->allocate(modelInfo.indices, modelInfo.nbIndices * sizeof(unsigned int));

        OpenGLMesh.VAO->release();

        initialized = true;
    }

    void TextureComponentSystem::init()
    {
        auto group = registerGroup<UiComponent, TextureComponent>();

        group->addOnGroup([](EntityRef entity) {
            LOG_INFO("Texture Component System", "Add entity " << entity->id << " to ui - tex group !");

            auto ui = entity->get<UiComponent>();
            auto tex = entity->get<TextureComponent>();

            auto tName = tex->textureName;

            auto sys = entity->world()->getSystem<TextureComponentSystem>();

            auto mesh = sys->getTextureMesh(ui->width, ui->height, tName);

            auto rTex = RenderableTexture{entity->id, ui, mesh};

            auto textureId = sys->masterRenderer->getTexture(tName);

            sys->tempRenderList[textureId].push_back(rTex);

            sys->changed = true;
        });
    }

    void TextureComponentSystem::onEvent(const UiComponentChangeEvent& event)
    {
        auto entity = ecsRef->getEntity(event.id);

        if(not entity->has<TextureComponent>())
            return;

        auto ui = entity->get<UiComponent>();

        // Todo check if the entity has a sentence text before trying to modify it
        auto tex = entity->get<TextureComponent>();

        auto tName = tex->textureName;

        auto sys = entity->world()->getSystem<TextureComponentSystem>();

        auto mesh = sys->getTextureMesh(ui->width, ui->height, tName);

        auto rTex = RenderableTexture{event.id, ui, mesh};

        LOG_MILE("Texture Component System", "Modification of id: " << entity->id << " texture");

        std::lock_guard<std::mutex> lock (sys->modificationMutex);

        auto textureId = sys->masterRenderer->getTexture(tName);

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

    void TextureComponentSystem::render()
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

    Mesh* TextureComponentSystem::getTextureMesh(float width, float height, const std::string& name)
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

    CompList<UiComponent, TextureComponent> makeUiTexture(EntitySystem *ecs, float width, float height, const std::string& name)
    {
        auto entity = ecs->createEntity();

        auto ui = ecs->attach<UiComponent>(entity);

        ui->setWidth(width);
        ui->setHeight(height);

        auto tex = ecs->attach<TextureComponent>(entity, name);

        return CompList<UiComponent, TextureComponent>(entity, ui, tex);
    }
}