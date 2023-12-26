#pragma once

#include "uiconstant.h"
// Todo only need to import /ecs/system ! modify accordingly
#include "ECS/entitysystem.h"
#include "serialization.h"

//Todo parenting, better anchoring
//TODO refactor all the struct that need to be a class and make the element private

//TODO make the UiComponent a class
//TODO create a destructor that go through the children and remove this ? or remove the child recursively ?
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

        // Type definition
    private:
        /**
         * @brief A struct describing the two anchor points needed to represent a corner
         */
        struct Corner
        {
            const UiSize* verticalAnchor;       ///< The vertical anchor point of the corner
            const UiSize* horizontalAnchor;     ///< The horizontal anchor point of the corner
        };

        // Public interface
    public:
        /** The position of the object */
        UiPosition pos;

        /** The width of the object */
        UiSize width = AnchorDir::Width;
        /** The height of the object */
        UiSize height = AnchorDir::Height;

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
        }

        inline static std::string getType() { return "UiComponent"; } 

        /**
         * @brief Destroy the Ui Component object
         */
        virtual ~UiComponent() { }

        // Setter methods for position and size properties
    public:
        inline void setX(const float& value)              { pos.x = value; update(); }
        inline void setY(const float& value)              { pos.y = value; update(); }
        inline void setZ(const float& value)              { pos.z = value; update(); }

        inline void setX(const UiSize& value)             { pos.x = value; update(); }
        inline void setY(const UiSize& value)             { pos.y = value; update(); }
        inline void setZ(const UiSize& value)             { pos.z = value; update(); }

        inline void setWidth(const float& value)          { width = value;  update(); }
        inline void setHeight(const float& value)         { height = value; update(); }

        inline void setWidth(const UiSize& value)         { width = value;  update(); }
        inline void setHeight(const UiSize& value)        { height = value; update(); }

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

        inline void fill(UiComponent *component)
        {
            topAnchor    = &component->top;
            rightAnchor  = &component->right;
            bottomAnchor = &component->bottom;
            leftAnchor   = &component->left;

            update();
        }

        inline void fill(const UiComponent& component)
        {
            topAnchor    = &component.top;
            rightAnchor  = &component.right;
            bottomAnchor = &component.bottom;
            leftAnchor   = &component.left;

            update();
        }

        // TODO add function for alignement with corner, vertical and horizontal center, center alignement
        // and fill

        // Public helper methods
    public:
        bool inBound(int x, int y) const;
        bool inBound(const constant::Vector2D& vec2) const;

        const bool& isVisible() const { return visible; }

        virtual void show() { this->visible = true; }
        virtual void hide() { this->visible = false; }

        virtual void render(MasterRenderer* masterRenderer);
        
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

    struct UiComponentChangeEvent
    {
        _unique_id id;
    };

    struct UiComponentSystem : public System<Own<UiComponent>, Listener<UiComponentInternalChangeEvent>, Listener<UiSizeChangeEvent>, NamedSystem>
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
                return comp.pos.x == x and
                       comp.pos.y == y and
                       comp.pos.z == z and
                       comp.width == w and
                       comp.height == h; 
            } 
        };

        UiComponentSystem() {}

        virtual std::string getSystemName() const override { return "Ui System"; }

        // Todo Set updated to true on add also !

        virtual void onEvent(const UiComponentInternalChangeEvent& event) override
        {
            std::lock_guard lock(m);

            LOG_INFO("Ui internals", "Entity " << event.parent << " is a parent of " << event.child);

            auto& vec = parentalMap[event.parent];
            
            auto it = std::find(vec.begin(), vec.end(), event.child);

            if(it == vec.end())
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
                
                if(it != parentalMap.end())
                {
                    vec = it->second;
                }

                // for (const auto& childId : vec)
                // {
                //     auto child = atEntity<UiComponent>(childId);

                //     // Todo set dirty only the child UiSize and not the whole ui component itself !

                //     if (child)
                //         child->setDirty();
                // }
            }
            
            for(auto& child : vec)
            {
                ecsRef->sendEvent(UiComponentChangeEvent{child});
            }
        }

        virtual void execute() override
        {
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