#pragma once

#include "ECS/entitysystem.h"

namespace pg
{
    // Todo finish this
    struct PositionComponent : public Ctor
    {
        PositionComponent() {}

        PositionComponent(const PositionComponent& other) :
            x(other.x), y(other.y), z(other.z), w(other.w), h(other.h), a(other.a),
            xAxisAnchored(other.xAxisAnchored), yAxisAnchored(other.yAxisAnchored),
            ecsRef(other.ecsRef), entityId(other.entityId)
        {}

        PositionComponent& operator=(const PositionComponent& other)
        {
            x = other.x;
            y = other.y;
            z = other.z;
            w = other.w;
            h = other.h;
            a = other.a;
            xAxisAnchored = other.xAxisAnchored;
            yAxisAnchored = other.yAxisAnchored;
            ecsRef = other.ecsRef;

            return *this;
        }

        virtual void onCreation(EntityRef entity) override
        {
            ecsRef = entity->world();

            entityId = entity.id;
        }

        float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f, h = 0.0f, a = 0.0f;

        // Todo if top and bottom anchor set then y axis is anchored (set Y and H do nothing), removing one of the anchor unset the axis anchoring
        bool xAxisAnchored = false, yAxisAnchored = false;

        EntitySystem *ecsRef = nullptr;

        _unique_id entityId = 0;
    };

    struct AnchorPositionComponent : public Ctor
    {
        enum class AnchorPoint : uint8_t
        {
            None = 0,
            Top,
            Left,
            Bottom,
            Right
        };

        virtual void onCreation(EntityRef entity) override
        {
            ecsRef = entity->world();

            entityId = entity.id;
        }

        AnchorPoint hasTopAnchor    = AnchorPoint::None;
        AnchorPoint hasLeftAnchor   = AnchorPoint::None;
        AnchorPoint hasBottomAnchor = AnchorPoint::None;
        AnchorPoint hasRightAnchor  = AnchorPoint::None;

        float topMargin    = 0.0f;
        float leftMargin   = 0.0f;
        float bottomMargin = 0.0f;
        float rightMargin  = 0.0f;

        _unique_id topAnchorId    = 0;
        _unique_id leftAnchorId   = 0;
        _unique_id bottomAnchorId = 0;
        _unique_id rightAnchorId  = 0;

        EntitySystem *ecsRef = nullptr;

        _unique_id entityId = 0;
    };

}