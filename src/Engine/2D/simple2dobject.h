#pragma once

#include "logger.h"

#include "UI/uisystem.h"

#include "Renderer/renderer.h"

#include "constant.h"

namespace pg
{
    enum class Shape2D : uint8_t
    {
        Triangle = 0,
        Square,
        Circle,
        None
    };

    struct Simple2DObject : public Ctor
    {
        Simple2DObject(const Shape2D& shape) : shape(shape) { }
        Simple2DObject(const Shape2D& shape, float width, float height, const constant::Vector3D& c) : shape(shape)
        {
            size.x = width;
            size.y = height;

            colors = c;
        }

        Simple2DObject(const Simple2DObject &rhs) : shape(rhs.shape), size(rhs.size), colors(rhs.colors), entity(rhs.entity) { }
        virtual ~Simple2DObject() {}

        inline static std::string getType() { return "Simple2DObject"; } 

        virtual void onCreation(EntityRef entity) { this->entity = entity; }

        Shape2D shape;

        // Todo be specific for each shape (rect = [width, height], circle = [origin, radius], triangle = [base, height] + rotation arg)
        constant::Vector2D size {10.0f, 10.0f};

        // Todo make the colors normalized (0.0f <-> 1.0f) to not do the division in the shader
        constant::Vector3D colors {255.0f, 255.0f, 255.0f};

        Entity *entity = nullptr;
    };

    template <>
    void serialize(Archive& archive, const Shape2D& value);

    template <>
    void serialize(Archive& archive, const Simple2DObject& value);

    template <>
    Shape2D deserialize(const UnserializedObject& serializedString);

    template <>
    Simple2DObject deserialize(const UnserializedObject& serializedString);

    struct Simple2DRenderCall
    {
        Simple2DRenderCall(const RenderCall& call) : call(call) {}

        RenderCall call;
    };

    struct Simple2DObjectSystem : public AbstractRenderer, System<Own<Simple2DObject>, Own<Simple2DRenderCall>, Ref<UiComponent>, Listener<EntityChangedEvent>, NamedSystem, InitSys>
    {
        Simple2DObjectSystem(MasterRenderer* masterRenderer) : AbstractRenderer(masterRenderer, RenderStage::Render) { }
        virtual ~Simple2DObjectSystem() { }

        virtual std::string getSystemName() const override { return "Shape 2D System"; }

        virtual void init() override;

        virtual void execute() override;

        RenderCall createRenderCall(CompRef<UiComponent> ui, CompRef<Simple2DObject> obj);

        virtual void onEvent(const EntityChangedEvent& event) override;

        uint64_t materialId = 0;
    };

    CompList<UiComponent, Simple2DObject> makeSimple2DShape(EntitySystem *ecs, const Shape2D& shape, float width, float height, const constant::Vector3D& colors);
}