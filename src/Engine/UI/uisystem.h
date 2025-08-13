#pragma once

#include "uiconstant.h"
// Todo only need to import /ecs/system ! modify accordingly
#include "ECS/entitysystem.h"
#include "serialization.h"
#include "pgconstant.h"
// Todo parenting, better anchoring
// TODO refactor all the struct that need to be a class and make the element private

// TODO make the UiComponent a class
// TODO create a destructor that go through the children and remove this ? or remove the child recursively ?

// Todo separate Position and Anchoring
namespace pg
{
    // Forward declaration
    namespace constant
    {
        struct Vector2D;
    }

    class MasterRenderer;
    struct UiComponentSystem;

    /**
     * @class UiComponent
     *
     * Base struct of all the ui components.
     * This struct all data about the position of the object
     * as well as a render function
     */
    class UiComponent : public Ctor
    {
    friend struct UiComponentSystem;
    friend struct ListView;
    friend struct Prefab;
        // Type definition
    private:
        /**
         * @brief A struct describing the two anchor points needed to represent a corner
         */
        struct Corner
        {
            UiSize verticalAnchor;       ///< The vertical anchor point of the corner
            UiSize horizontalAnchor;     ///< The horizontal anchor point of the corner

            bool operator==(const Corner& other)
            {
                return areAlmostEqual(verticalAnchor, other.verticalAnchor) and areAlmostEqual(horizontalAnchor, other.horizontalAnchor);
            }

            bool operator!=(const Corner& other)
            {
                return not (*this == other);
            }
        };

        // Public interface
    public:
        /** The position of the object */
        UiPosition pos;

        /** The width of the object */
        UiSize width = AnchorDir::Width;
        /** The height of the object */
        UiSize height = AnchorDir::Height;

        float rotation = 0.0f;

        /** The bounding box of the object */
        const UiFrame frame = UiFrame(pos.x, pos.y, pos.z, width, height);

        // The 4 anchors points of the object
        /** Top anchor points of the object */
        // Todo make this const
        UiSize top    = pos.y;
        /** Right anchor points of the object */
        UiSize right  = pos.x + width;
        /** Bottom anchor points of the object */
        UiSize bottom = pos.y + height;
        /** Left anchor points of the object */
        UiSize left   = pos.x;

        // The 4 corner points of the object
        /** Top left corner of the object */
        const Corner topLeft     = {&top,    &left};
        /** Top right corner of the object */
        const Corner topRight    = {&top,    &right};
        /** Bottom left corner of the object */
        const Corner bottomLeft  = {&bottom, &left};
        /** Bottom right corner of the object */
        const Corner bottomRight = {&bottom, &right};

        // The margin to the given anchor point
        /** The margin from the top anchor point */
        UiSize topMargin = AnchorDir::TMargin;
        /** The margin from the right anchor point */
        UiSize rightMargin = AnchorDir::RMargin;
        /** The margin from the bottom anchor point */
        UiSize bottomMargin = AnchorDir::BMargin;
        /** The margin from the left anchor point */
        UiSize leftMargin = AnchorDir::LMargin;

        // The top left point of the clip rectangle of this component
        Corner clipTopLeft;

        // The bottom right point of the clip rectangle of this component
        Corner clipBottomRight;

        /**
         * @brief Construct a new Ui Component object
         *
         * This construct an empty ui component at coord (0.0f, 0.0f, 0.0f) of size 0, 0
         */
        UiComponent()
        {
            top = AnchorDir::Top;
            right = AnchorDir::Right;
            bottom = AnchorDir::Bottom;
            left = AnchorDir::Left;
        }

        /**
         * @brief Construct a new Ui Component object
         *
         * @param frame The reference frame for this object
         *
         * This construct an ui component from a given reference
         * The position and size of the newly created object is automatically updated on the fly
         */
        UiComponent(const UiFrame& frame) : pos(&frame.pos), width(&frame.w), height(&frame.h)
        {
            top = AnchorDir::Top;
            right = AnchorDir::Right;
            bottom = AnchorDir::Bottom;
            left = AnchorDir::Left;
        }

        /**
         * @brief Construct a new Ui Component object
         *
         * @param rhs Another UiComponent
         *
         * This creates a copy of the rhs ui component
         */
        UiComponent(const UiComponent& rhs);

        /**
         * @brief Add id info to anchors if created in a ECS
         *
         * @param entity The entity that hold this component
         */
        virtual void onCreation(EntityRef entity) override
        {
            ecsRef = entity->world();

            entityId = entity->id;

            top.setEntity(entityId, ecsRef);
            right.setEntity(entityId, ecsRef);
            bottom.setEntity(entityId, ecsRef);
            left.setEntity(entityId, ecsRef);

            topAnchor.setEntity(entityId, ecsRef);
            rightAnchor.setEntity(entityId, ecsRef);
            bottomAnchor.setEntity(entityId, ecsRef);
            leftAnchor.setEntity(entityId, ecsRef);

            pos.setEntity(entityId, ecsRef);

            width.setEntity(entityId, ecsRef);
            height.setEntity(entityId, ecsRef);

            topMargin.setEntity(entityId, ecsRef);
            leftMargin.setEntity(entityId, ecsRef);
            rightMargin.setEntity(entityId, ecsRef);
            bottomMargin.setEntity(entityId, ecsRef);

            update();
        }

        inline static std::string getType() { return "UiComponent"; }

        /**
         * @brief Destroy the Ui Component object
         */
        virtual ~UiComponent() { }

        // Setter methods for position and size properties
    public:
        inline void setX(const float& value)              { if (areNotAlmostEqual(pos.x, value)) { pos.x = value; update(); } }
        inline void setY(const float& value)              { if (areNotAlmostEqual(pos.y, value)) { pos.y = value; update(); } }
        inline void setZ(const float& value)              { if (areNotAlmostEqual(pos.z, value)) { pos.z = value; update(); } }

        inline void setX(const UiSize& value)             { pos.x = value; update(); }
        inline void setY(const UiSize& value)             { pos.y = value; update(); }
        inline void setZ(const UiSize& value)             { pos.z = value; update(); }

        inline void setWidth(const float& value)          { width = value;  update(); }
        inline void setHeight(const float& value)         { height = value; update(); }

        inline void setWidth(const UiSize& value)         { width = value;  update(); }
        inline void setHeight(const UiSize& value)        { height = value; update(); }

        inline void setRotation(const float& value)       { rotation = value; update(); }

        inline void setVisibility(const bool value)       { if (visible != value) { visible = value; update(); } }

        // Todo manage all case where has...Anchor get removed : (Add user function to remove anchor, when setting xyz ...)
        inline void setTopAnchor(const UiSize& anchor)    { topAnchor = &anchor;    hasTopAnchor = true;    update(); }
        inline void setRightAnchor(const UiSize& anchor)  { rightAnchor = &anchor;  hasRightAnchor = true;  update(); }
        inline void setBottomAnchor(const UiSize& anchor) { bottomAnchor = &anchor; hasBottomAnchor = true; update(); }
        inline void setLeftAnchor(const UiSize& anchor)   { leftAnchor = &anchor;   hasLeftAnchor = true;   update(); }

        inline void setTopMargin(const int& value)        { topMargin = value;    update(); }
        inline void setRightMargin(const int& value)      { rightMargin = value;  update(); }
        inline void setBottomMargin(const int& value)     { bottomMargin = value; update(); }
        inline void setLeftMargin(const int& value)       { leftMargin = value;   update(); }

        inline void setTopMargin(const UiSize& value)     { topMargin = value;    update(); }
        inline void setRightMargin(const UiSize& value)   { rightMargin = value;  update(); }
        inline void setBottomMargin(const UiSize& value)  { bottomMargin = value; update(); }
        inline void setLeftMargin(const UiSize& value)    { leftMargin = value;   update(); }

        // Used to set a custom clip rect
        inline void setClipRect(const Corner& topLeft, const Corner& bottomRight) { isClippedToWindow = false; clipTopLeft = topLeft; clipBottomRight = bottomRight; update(); }

        // Set back the clip rect to the window (instead of a custom one)
        inline void clipBackToWindow() { isClippedToWindow = true; }

        inline void fill(UiComponent *component)
        {
            topAnchor    = &component->top;
            rightAnchor  = &component->right;
            bottomAnchor = &component->bottom;
            leftAnchor   = &component->left;

            hasTopAnchor = true;
            hasRightAnchor = true;
            hasBottomAnchor = true;
            hasLeftAnchor = true;

            update();
        }

        inline void fill(const UiComponent& component)
        {
            topAnchor    = &component.top;
            rightAnchor  = &component.right;
            bottomAnchor = &component.bottom;
            leftAnchor   = &component.left;

            hasTopAnchor = true;
            hasRightAnchor = true;
            hasBottomAnchor = true;
            hasLeftAnchor = true;

            update();
        }

        // TODO add function for alignement with corner, vertical and horizontal center, center alignement
        // and fill

        // Public helper methods
    public:
        bool inBound(float x, float y) const;
        bool inBound(const constant::Vector2D& vec2) const;

        bool inClipBound(float x, float y) const;
        bool inClipBound(const constant::Vector2D& vec2) const;

        inline bool isWindowClipped() const { return isClippedToWindow; }

        const bool& isVisible() const { return visible; }

        void show() { if (not this->visible) { this->visible = true; update(); } }
        void hide() { if (this->visible) { this->visible = false; update(); } }

        void update();

    protected:
        /** Flag indicating if the uicomponent should be rendered. */
        bool visible = true;

    public:
        bool hasTopAnchor = false;
        bool hasRightAnchor = false;
        bool hasBottomAnchor = false;
        bool hasLeftAnchor = false;

    private:
        friend void serialize<>(Archive& archive, const UiComponent& value);

        // Pointer to anchors where this object is tied

        /** Pointer to the top attached anchor */
        UiSize topAnchor;
        /** Pointer to the right attached anchor */
        UiSize rightAnchor;
        /** Pointer to the bottom attached anchor */
        UiSize bottomAnchor;
        /** Pointer to the left attached anchor */
        UiSize leftAnchor;

        bool isClippedToWindow = true;

        EntitySystem* ecsRef = nullptr;

        _unique_id entityId = 0;
    };

    template <>
    void serialize(Archive& archive, const AnchorDir& value);

    template <>
    void serialize(Archive& archive, const UiPosition& value);

    template <>
    void serialize(Archive& archive, const UiFrame& value);

    template <>
    void serialize(Archive& archive, const UiComponent& value);

    struct UiComponentSystem : public System<Own<UiComponent>, Listener<ResizeEvent>, Listener<UiComponentInternalChangeEvent>, Listener<UiSizeChangeEvent>>
    {
        struct UiOldValue
        {
            float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f, h = 0.0f;

            inline void operator=(const UiComponent& comp)
            {
                x = comp.pos.x;
                y = comp.pos.y;
                z = comp.pos.z;
                w = comp.width;
                h = comp.height;
            }

            inline bool isEqual(const UiComponent& comp) const
            {
                return areAlmostEqual(comp.pos.x, x) and
                       areAlmostEqual(comp.pos.y, y) and
                       areAlmostEqual(comp.pos.z, z) and
                       areAlmostEqual(comp.width, w) and
                       areAlmostEqual(comp.height, h);
            }
        };

        UiComponentSystem() {}

        virtual std::string getSystemName() const override { return "Ui System"; }

        // Todo Set updated to true on add also !

        virtual void onEvent(const ResizeEvent&) override
        {
            std::lock_guard lock(m);

            LOG_INFO("Ui internals", "Window resizing");

            // Todo

            // for (const auto& comp : view<UiComponent>())
            // {
            //     comp->updateWindowRect({0.0f, 0.0f}, {event.width, event.height});
            // }
        }

        virtual void onEvent(const UiComponentInternalChangeEvent& event) override
        {
            std::lock_guard lock(m);

            LOG_MILE("Ui internals", "Entity " << event.parent << " is a parent of " << event.child);

            auto& vec = parentalMap[event.parent];

            auto it = std::find(vec.begin(), vec.end(), event.child);

            if (it == vec.end())
                vec.push_back(event.child);
        }

        virtual void onEvent(const UiSizeChangeEvent& event) override
        {
            std::unordered_map<_unique_id, std::vector<_unique_id>>::iterator it;
            std::vector<_unique_id> vec;

            // Todo update all the parent map at once
            {
                std::lock_guard lock(m);

                it = parentalMap.find(event.id);

                if (it != parentalMap.end())
                {
                    vec = it->second;
                }

                // Put all children in a set to only call the change event once per id
                for (auto& child : vec)
                {
                    updatedId.insert(child);
                }

                updated = true;

                // for (const auto& childId : vec)
                // {
                //     auto child = atEntity<UiComponent>(childId);

                //     // Todo set dirty only the child UiSize and not the whole ui component itself !

                //     if (child)
                //         child->setDirty();
                // }
            }

            // for(auto& child : vec)
            // {
            //     ecsRef->sendEvent(UiComponentChangeEvent{child});
            // }
        }

        virtual void execute() override
        {
            if (updated)
            {
                std::lock_guard lock(m);

                for (auto& child : updatedId)
                {
                    ecsRef->sendEvent(EntityChangedEvent{child});
                }

                updatedId.clear();

                updated = false;
            }
            // if(updated)
            // {
            //     for(auto comp : view<UiComponent>())
            //     {
            //         if(not oldMap[comp->entityId].isEqual(*comp))
            //         {
            //             oldMap[comp->entityId] = *comp;
            //             ecsRef->sendEvent(UiComponentChangeEvent{comp->entityId, comp});
            //         }
            //     }

            //     updated = false;
            // }
        }

        std::set<_unique_id> updatedId;

        std::unordered_map<_unique_id, std::vector<_unique_id>> parentalMap;

        std::mutex m;

        bool updated = false;
    };

    template <typename LoaderId>
    struct LoaderRenderComponent : public UiComponent
    {
        LoaderRenderComponent(const LoaderId *id) : id(id) {}
        LoaderRenderComponent(const LoaderRenderComponent& rhs);

        //virtual void render(MasterRenderer* masterRenderer) { renderer(masterRenderer, this); }

        const LoaderId *id;
    };

    template <typename LoaderId>
    LoaderRenderComponent<LoaderId>::LoaderRenderComponent(const LoaderRenderComponent &rhs) : UiComponent(rhs), id(rhs.id)
    {
        update();
    }

}