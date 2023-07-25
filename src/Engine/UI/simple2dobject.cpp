#define STB_IMAGE_IMPLEMENTATION

#include "simple2dobject.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "logger.h"

#include "openglobject.h"

namespace pg
{
    namespace
    {
        static constexpr char const * DOM = "Shape 2D";
    }

    void Shape2DMesh::generateMesh()
    {
        LOG_THIS_MEMBER("Shape 2D Mesh");

        OpenGLMesh.initialize();

        OpenGLMesh.VAO->bind();

        OpenGLMesh.VBO->bind();
        OpenGLMesh.VBO->setUsagePattern(OpenGLBuffer::StaticDraw);
        OpenGLMesh.VBO->allocate(modelInfo.vertices, modelInfo.nbVertices * sizeof(float));

        // Position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        OpenGLMesh.EBO->bind();
        OpenGLMesh.EBO->setUsagePattern(OpenGLBuffer::StaticDraw);
        OpenGLMesh.EBO->allocate(modelInfo.indices, modelInfo.nbIndices * sizeof(unsigned int));

        OpenGLMesh.VAO->release();

        initialized = true;
    }

    void Simple2DObjectSystem::init()
    {
        auto group = registerGroup<UiComponent, Simple2DObject>();

        group->addOnGroup([](EntityRef entity) {
            LOG_INFO("Simple 2D Object System", "Add entity " << entity->id << " to ui - 2d shape group !");

            auto ui = entity->get<UiComponent>();
            auto shape = entity->get<Simple2DObject>();

            auto sys = entity->world()->getSystem<Simple2DObjectSystem>();

            auto mesh = sys->get2DMesh(*shape);

            auto rTex = RenderableTexture{entity->id, ui, mesh};

            sys->tempRenderList[static_cast<unsigned int>(shape->shape)].push_back(rTex);

            sys->changed = true;
        });
    }

    void Simple2DObjectSystem::render()
    {
        LOG_THIS(DOM);
    
        auto rTable = masterRenderer->getParameter();
        const int screenWidth = rTable["ScreenWidth"];
        const int screenHeight = rTable["ScreenHeight"];

        glm::mat4 projection = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 scale = glm::mat4(1.0f);

        scale = glm::scale(scale, glm::vec3(2.0f / screenWidth, 2.0f / screenHeight, 1.0f));

        auto shaderProgram = masterRenderer->getShader("2DShapes");

        for(const auto& renderList : currentRenderList)
        {
            shaderProgram->bind();

            shaderProgram->setUniformValue("projection", projection);
            shaderProgram->setUniformValue("model", model);
            shaderProgram->setUniformValue("scale", scale);

            // Todo combine all the call to the same texture into a single draw call using instanced rendering
            for(const auto& renderableTexture : renderList.second)
            {
                UiComponent *ui = renderableTexture.uiRef;

                auto mesh = renderableTexture.meshRef;

                if(not ui->isVisible() or not mesh)
                    continue;

                view = glm::mat4(1.0f);
                view = glm::translate(view, glm::vec3(-1.0f + 2.0f * static_cast<UiSize>(ui->pos.x) / screenWidth, 1.0f + 2.0f * -static_cast<UiSize>(ui->pos.y) / screenHeight, 0.0f));

                shaderProgram->setUniformValue("view", view);

                auto colors = static_cast<Shape2DMesh*>(mesh)->colors;

                shaderProgram->setUniformValue("colors", glm::vec3(colors.x / 255.0f, colors.y / 255.0f, colors.z / 255.0f));

                mesh->bind();
                glDrawElements(GL_TRIANGLES, mesh->modelInfo.nbIndices, GL_UNSIGNED_INT, 0);
            }
        }

        shaderProgram->release();
    }

    Mesh* Simple2DObjectSystem::get2DMesh(const Simple2DObject& shape)
    {
        LOG_THIS_MEMBER("MeshBuilder");

        std::string meshName = "";

        switch (shape.shape)
        {
        case Shape2D::Triangle:
            meshName += "_triangle_";
            break;

        case Shape2D::Square:
            meshName += "_square_";
            break;

        case Shape2D::Circle:
            meshName += "_circle_";
            break; 
        
        default:
            LOG_ERROR(DOM, "Unknown shape, no mesh generated");
            return nullptr;
            break;
        }

        // Todo this will become irelevant once we support instanced rendering
        meshName += "_" + std::to_string(shape.size.x) + "_" + std::to_string(shape.size.y) + "_" + std::to_string(shape.colors.x) + "_" + std::to_string(shape.colors.y) + "_" + std::to_string(shape.colors.z);

        const auto& it = meshes.find(meshName);

        if(it != meshes.end())
        {
            // Todo increment the number of time this mesh is used
            // m_meshes[meshName].count++;

            return it->second;   
        }

        LOG_MILE("MeshBuilder", "Creating a new 2D shape mesh: " << meshName);

        auto mesh = new Shape2DMesh();

        float base, height, width;

        switch (shape.shape)
        {
        case Shape2D::Triangle:
            mesh->modelInfo.nbVertices = 9;
            mesh->modelInfo.nbIndices = 3;

            if(mesh->modelInfo.vertices != nullptr)
                delete[] mesh->modelInfo.vertices;
            if(mesh->modelInfo.indices != nullptr)
                delete[] mesh->modelInfo.indices;

            mesh->modelInfo.vertices = new float [mesh->modelInfo.nbVertices];
            mesh->modelInfo.indices = new unsigned int [mesh->modelInfo.nbIndices];

            base = shape.size.x;
            height = shape.size.y;

            mesh->modelInfo.vertices[0] = base / 2.0f; mesh->modelInfo.vertices[1] =  0.0f;   mesh->modelInfo.vertices[2] = 0.0f;
            mesh->modelInfo.vertices[3] = base;        mesh->modelInfo.vertices[4] = -height; mesh->modelInfo.vertices[5] = 0.0f;
            mesh->modelInfo.vertices[6] = 0.0f;        mesh->modelInfo.vertices[7] = -height; mesh->modelInfo.vertices[8] = 0.0f;

            mesh->modelInfo.indices[0] = 0; mesh->modelInfo.indices[1] = 1; mesh->modelInfo.indices[2] = 2;
            break;

        // If the shape is a square we keep the basic square info
        case Shape2D::Square:
            mesh->modelInfo.nbVertices = 12;
            mesh->modelInfo.nbIndices = 6;

            if(mesh->modelInfo.vertices != nullptr)
                delete[] mesh->modelInfo.vertices;
            if(mesh->modelInfo.indices != nullptr)
                delete[] mesh->modelInfo.indices;

            mesh->modelInfo.vertices = new float [mesh->modelInfo.nbVertices];
            mesh->modelInfo.indices = new unsigned int [mesh->modelInfo.nbIndices];

            width = shape.size.x;
            height = shape.size.y;

            mesh->modelInfo.vertices[0] = 0.0f;  mesh->modelInfo.vertices[1]   =  0.0f;   mesh->modelInfo.vertices[2]  = 0.0f;
            mesh->modelInfo.vertices[3] = width; mesh->modelInfo.vertices[4]   =  0.0f;   mesh->modelInfo.vertices[5]  = 0.0f;
            mesh->modelInfo.vertices[6] = 0.0f;   mesh->modelInfo.vertices[7]  = -height; mesh->modelInfo.vertices[8]  = 0.0f;
            mesh->modelInfo.vertices[9] = width;  mesh->modelInfo.vertices[10] = -height; mesh->modelInfo.vertices[11] = 0.0f;

            mesh->modelInfo.indices[0] = 0; mesh->modelInfo.indices[1] = 1; mesh->modelInfo.indices[2] = 2;
            mesh->modelInfo.indices[3] = 1; mesh->modelInfo.indices[4] = 2; mesh->modelInfo.indices[5] = 3;
            break;

        // Todo support circles
        case Shape2D::Circle:
            LOG_ERROR(DOM, "Circle are not supported yet, no mesh generated");
            return nullptr;
            break; 
        
        default:
            LOG_ERROR(DOM, "Unknown shape, no mesh generated");
            return nullptr;
            break;
        }

        mesh->colors.x = shape.colors.x;
        mesh->colors.y = shape.colors.y;
        mesh->colors.z = shape.colors.z;

        meshes.emplace(meshName, mesh);

        return mesh;
    }

    CompList<UiComponent, Simple2DObject> makeSimple2DShape(EntitySystem *ecs, const Shape2D& shape, float width, float height, const constant::Vector3D& colors)
    {
        auto entity = ecs->createEntity();

        auto ui = ecs->attach<UiComponent>(entity);

        ui->setWidth(width);
        ui->setHeight(height);

        auto tex = ecs->attach<Simple2DObject>(entity, shape, width, height, colors);

        return CompList<UiComponent, Simple2DObject>(entity, ui, tex);
    }
}
