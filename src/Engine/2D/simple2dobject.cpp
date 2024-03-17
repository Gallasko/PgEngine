#define STB_IMAGE_IMPLEMENTATION

#include "simple2dobject.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "logger.h"

#include "Helpers/openglobject.h"

namespace pg
{
    namespace
    {
        static constexpr char const * DOM = "Shape 2D";
    }

    void Shape2DMesh::generateMesh()
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

        openGLMesh.EBO->bind();
        openGLMesh.EBO->setUsagePattern(OpenGLBuffer::StaticDraw);
        openGLMesh.EBO->allocate(modelInfo.indices, modelInfo.nbIndices * sizeof(unsigned int));

        openGLMesh.VAO->release();

        initialized = true;
    }

    void Simple2DObjectSystem::SimpleSquareMesh::generateMesh()
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

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribDivisor(1, 1); // tell OpenGL this is an instanced vertex attribute.
        glVertexAttribDivisor(2, 1); // tell OpenGL this is an instanced vertex attribute.
        glVertexAttribDivisor(3, 1); // tell OpenGL this is an instanced vertex attribute.

        openGLMesh.EBO->bind();
        openGLMesh.EBO->setUsagePattern(OpenGLBuffer::StaticDraw);
        openGLMesh.EBO->allocate(modelInfo.indices, modelInfo.nbIndices * sizeof(unsigned int));

        openGLMesh.VAO->release();

        initialized = true;
    }

    Simple2DObjectSystem::SimpleSquareMesh::~SimpleSquareMesh()
    { 
        LOG_THIS_MEMBER("Shape 2D Mesh");

        if (instanceVBO)
            delete instanceVBO;
    }

    void Simple2DObjectSystem::init()
    {
        auto group = registerGroup<UiComponent, Simple2DObject>();

        group->addOnGroup([](EntityRef entity) {
            LOG_INFO("Simple 2D Object System", "Add entity " << entity->id << " to ui - 2d shape group !");

            auto ui = entity->get<UiComponent>();
            auto shape = entity->get<Simple2DObject>();

            auto sys = entity->world()->getSystem<Simple2DObjectSystem>();

            sys->addElement(ui, shape);
        });

        group->removeOfGroup([](EntitySystem* ecsRef, _unique_id id) {
            LOG_INFO("Simple 2D Object System", "Remove entity " << id << " of ui - 2d shape group !");

            auto sys = ecsRef->getSystem<Simple2DObjectSystem>();

            sys->removeElement(id);
        });
    }

    void Simple2DObjectSystem::render()
    {
        LOG_THIS_MEMBER(DOM);

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

            if (sizeChanged)
            {
                basicSquareMesh.bind();

                basicSquareMesh.instanceVBO->allocate(bufferData, size * nbAttributes * sizeof(float));
                
                sizeChanged = false;
            }
            else
            {
                basicSquareMesh.bind();

                basicSquareMesh.instanceVBO->allocate(bufferData, size * nbAttributes * sizeof(float));
            }

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

        auto shaderProgram = masterRenderer->getShader("2DShapes");

        shaderProgram->bind();

        shaderProgram->setUniformValue("sWidth", static_cast<float>(screenWidth));
        shaderProgram->setUniformValue("sHeight", static_cast<float>(screenHeight));

        shaderProgram->setUniformValue("projection", projection);
        shaderProgram->setUniformValue("model", model);
        shaderProgram->setUniformValue("scale", scale);
        shaderProgram->setUniformValue("view", view);

        basicSquareMesh.bind();
        glDrawElementsInstanced(GL_TRIANGLES, basicSquareMesh.modelInfo.nbIndices, GL_UNSIGNED_INT, 0, nbElements);

        shaderProgram->release();
    }

    void Simple2DObjectSystem::addElement(const CompRef<UiComponent>& ui, const CompRef<Simple2DObject>& obj)
    {
        LOG_INFO("Simple 2D Object System", "Add element " << ui.entityId << " to buffer !");

        {
            std::lock_guard<std::mutex> lock (modificationMutex);

            idToIndexMap[ui.entityId] = elementIndex;

            auto currentIndex = elementIndex++;

            auto visible = ui->isVisible();
            
            bufferData[currentIndex * nbAttributes + 0] = ui->pos.x;
            bufferData[currentIndex * nbAttributes + 1] = ui->pos.y;
            bufferData[currentIndex * nbAttributes + 2] = ui->pos.z;

            bufferData[currentIndex * nbAttributes + 3] = ui->width;
            bufferData[currentIndex * nbAttributes + 4] = ui->height;

            bufferData[currentIndex * nbAttributes + 5] = obj->colors.x;
            bufferData[currentIndex * nbAttributes + 6] = obj->colors.y;
            bufferData[currentIndex * nbAttributes + 7] = obj->colors.z;

            if (visible)
            {
                auto index = visibleElements++;

                if (visibleElements != elementIndex)
                {
                    swapIndex(currentIndex, index);
                }
            }

            if(currentIndex + 1 >= currentSize)
            {
                increaseSize();
            }

            changed = true;
        }
    }

    void Simple2DObjectSystem::onEvent(const UiComponentChangeEvent& event)
    {
        size_t index;

        auto entity = ecsRef->getEntity(event.id);
        
        if(not entity or not entity->has<UiComponent>())
            return; 

        auto ui = entity->get<UiComponent>();

        float x = ui->pos.x;
        float y = ui->pos.y;
        float z = ui->pos.z;

        float w = ui->width;
        float h = ui->height;

        bool visible = ui->isVisible();

        {
            std::lock_guard<std::mutex> lock(modificationMutex);

            auto it = idToIndexMap.find(event.id);
    
            if (it == idToIndexMap.end())
                return;

            index = it->second;

            bufferData[index * nbAttributes + 0] = x;
            bufferData[index * nbAttributes + 1] = y;
            bufferData[index * nbAttributes + 2] = z;

            bufferData[index * nbAttributes + 3] = w;
            bufferData[index * nbAttributes + 4] = h;

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

            changed = true;
        }

        // bufferData[index * nbAttributes + 5] = obj->colors.x;
        // bufferData[index * nbAttributes + 6] = obj->colors.y;
        // bufferData[index * nbAttributes + 7] = obj->colors.z;
    }

    void Simple2DObjectSystem::updateMeshes()
    {
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
