#pragma once

#include "ECS/system.h"

#include "pgconstant.h"

namespace pg
{
    enum class AnchorType : uint8_t
    {
        None = 0,
        Top,
        Right,
        Bottom,
        Left,
        X,
        Y,
        Z,
        Width,
        Height,
        TMargin,
        RMargin,
        BMargin,
        LMargin,
        VerticalCenter,
        HorizontalCenter
    };

    struct PositionComponentChangedEvent
    {
        _unique_id id = 0;
    };

    enum class PosOpType
    {
        None,
        Add,
        Sub,
        Mul,
        Div
    };

    // AnchorType to string and string to AnchorType maps
    extern const std::map<AnchorType, std::string> AnchorTypeToStringMap;
    extern const std::map<std::string, AnchorType> StringToAnchorTypeMap;

    // PosOpType to string and string to PosOpType maps
    extern const std::map<PosOpType, std::string> PosOpTypeToStringMap;
    extern const std::map<std::string, PosOpType> StringToPosOpTypeMap;

    inline static std::string getType() { return "UiComponent"; }

    struct PosConstrain
    {
        _unique_id id = 0;
        AnchorType type = AnchorType::None;

        PosOpType opType = PosOpType::None;
        float opValue = 0.0f;

        inline static std::string getType() { return "PosContrain"; }
    };

    struct PosAnchor
    {
        _unique_id id = 0;
        AnchorType type = AnchorType::None;
        float value = 0.0f;

        inline static std::string getType() { return "PosAnchor"; }
    };

    struct ParentingEvent
    {
        _unique_id parent = 0;
        _unique_id child = 0;
    };

    struct ClearParentingEvent
    {
        _unique_id parent = 0;
        _unique_id id = 0;
    };

    // Forward declaration
    struct PositionComponent;

    // Todo add a Dtor that remove any parenting
    // Be careful on edge case such as being anchored and clipped at the same time to the same entity
    // Need to count the number of time a child is parented to another entity
    struct UiAnchor : public Component, Dtor
    {
        DEFAULT_COMPONENT_MEMBERS(UiAnchor)

        // Current Anchor of this component
        PosAnchor top;
        PosAnchor left;
        PosAnchor right;
        PosAnchor bottom;

        // Basic cardinal anchors
        bool hasTopAnchor = false;
        bool hasLeftAnchor = false;
        bool hasRightAnchor = false;
        bool hasBottomAnchor = false;

        PosAnchor topAnchor;
        PosAnchor leftAnchor;
        PosAnchor rightAnchor;
        PosAnchor bottomAnchor;

        // Advanced cardinal anchors
        PosAnchor verticalCenter;
        PosAnchor horizontalCenter;

        bool hasVerticalCenter = false;
        bool hasHorizontalCenter = false;

        PosAnchor verticalCenterAnchor;
        PosAnchor horizontalCenterAnchor;

        // Cardinal margins
        float topMargin = 0.0f;
        float leftMargin = 0.0f;
        float rightMargin = 0.0f;
        float bottomMargin = 0.0f;

        // Constrains
        bool hasWidthConstrain = false;
        bool hasHeightConstrain = false;
        bool hasZConstrain = false;

        PosConstrain widthConstrain;
        PosConstrain heightConstrain;
        PosConstrain zConstrain;

        void setTopAnchor(const PosAnchor& anchor);
        void clearTopAnchor();

        void setLeftAnchor(const PosAnchor& anchor);
        void clearLeftAnchor();

        void setRightAnchor(const PosAnchor& anchor);
        void clearRightAnchor();

        void setBottomAnchor(const PosAnchor& anchor);
        void clearBottomAnchor();

        void setVerticalCenter(const PosAnchor& anchor);
        void clearVerticalCenter();

        void setHorizontalCenter(const PosAnchor& anchor);
        void clearHorizontalCenter();

        void fillIn(const UiAnchor& anchor);
        void fillIn(const UiAnchor* anchor);
        void centeredIn(const UiAnchor& anchor);
        void centeredIn(const UiAnchor* anchor);

        void clearAnchors();

        void setTopMargin(float value);
        void setLeftMargin(float value);
        void setRightMargin(float value);
        void setBottomMargin(float value);

        void setWidthConstrain(const PosConstrain& constrain);
        void setHeightConstrain(const PosConstrain& constrain);
        void setZConstrain(const PosConstrain& constrain);
        // Todo add visibility constrain

        // Todo add function to handle center, vertical and horizontal center alignment

        virtual void onCreation(EntityRef entity) override;

        virtual void onDeletion(EntityRef entity) override;

        void updateAnchor(bool hasAnchor, PosAnchor& anchor);

        bool update(CompRef<PositionComponent> positionComp);

        inline static std::string getType() { return "UiAnchor"; }
    };

    // Todo add a Dtor that remove any parenting
    struct ClippedTo : public Ctor
    {
        ClippedTo(_unique_id clipperId) : clipperId(clipperId) {}
        ClippedTo(const ClippedTo& other) : clipperId(other.clipperId), id(other.id), ecsRef(other.ecsRef) {}

        ClippedTo& operator=(const ClippedTo& other)
        {
            clipperId = other.clipperId;

            ecsRef = other.ecsRef;
            id = other.id;

            return *this;
        }

        virtual void onCreation(EntityRef entity) override;

        void setNewClipper(_unique_id clipperId);

        _unique_id clipperId;

        _unique_id id;

        EntitySystem *ecsRef = nullptr;
    };

    struct PositionComponent : public Ctor
    {
        // Todo make a basic constructor
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

        float width = 0.0f;
        float height = 0.0f;

        float rotation = 0.0f;

        bool visible = true;
        bool observable = true;

        virtual void onCreation(EntityRef entity) override;

        void setX(float x);
        void setY(float y);
        void setZ(float z);

        void setWidth(float width);
        void setHeight(float height);

        void setRotation(float rotation);

        void setVisibility(bool visible);
        void setObservable(bool observable);

        bool isVisible() const { return visible; }
        bool isObservable() const { return observable; }
        bool isRenderable() const { return visible and observable; }

        bool updatefromAnchor(const UiAnchor& anchor);

        inline static std::string getType() { return "PositionComponent"; }

        // Private:

        _unique_id id = 0;

        EntitySystem *ecsRef = nullptr;
    };

    template <>
    void serialize(Archive& archive, const PositionComponent& value);

    template <>
    void serialize(Archive& archive, const UiAnchor& value);

    template <>
    void serialize(Archive& archive, const PosAnchor& value);

    template <>
    void serialize(Archive& archive, const PosConstrain& value);

    template <>
    PositionComponent deserialize(const UnserializedObject& serializedString);

    template <>
    UiAnchor deserialize(const UnserializedObject& serializedString);

    template <>
    PosAnchor deserialize(const UnserializedObject& serializedString);

    template <>
    PosConstrain deserialize(const UnserializedObject& serializedString);

    // Todo add Listener<ResizeEvent>,
    struct PositionComponentSystem : public System<Own<PositionComponent>, Own<UiAnchor>, Own<ClippedTo>, Listener<ParentingEvent>, Listener<ClearParentingEvent>, QueuedListener<PositionComponentChangedEvent>>
    {
        virtual std::string getSystemName() const override { return "Position System"; }

        virtual void onEvent(const ParentingEvent& event) override
        {
            parentalMap[event.parent].insert(event.child);
        }

        virtual void onEvent(const ClearParentingEvent& event) override
        {
            parentalMap[event.parent].erase(event.id);
        }

        virtual void onProcessEvent(const PositionComponentChangedEvent& event) override
        {
            if (not changedIds.count(event.id))
            {
                changedIds.insert(event.id);
                pushChildrenInChange(changedIds, event.id);
            }
        }

        void pushChildrenInChange(std::set<_unique_id>& set, _unique_id parentId);

        virtual void execute() override;

        std::unordered_map<_unique_id, std::set<_unique_id>> parentalMap;

        std::set<_unique_id> changedIds;

        bool updated = false;
    };

    template <typename Type>
    CompList<PositionComponent, UiAnchor> makeAnchoredPosition(Type *ecs)
    {
        auto entity = ecs->createEntity();

        auto ui = ecs->template attach<PositionComponent>(entity);

        auto anchor = ecs->template attach<UiAnchor>(entity);

        return CompList<PositionComponent, UiAnchor>(entity, ui, anchor);
    }

    bool inBound(EntityRef entity, float x, float y);

    bool inClipBound(EntityRef entity, float x, float y);
}