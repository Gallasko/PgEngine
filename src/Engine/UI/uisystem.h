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

    struct UiComponentInternalChangeEvent { _unique_id parent, child; };

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
        // Todo use this struct instead of a plain uisize to avoid possible problems with user
        // passing a plain float or a deletable uisize where an anchor was expected
        // struct Anchor
        // {
        //     UiSize anchorPoint;
        // };
        
        /**
         * @brief A struct describing the two anchor points needed to represent a corner
         */
        struct Corner
        {
            const UiAnchor* verticalAnchor;       ///< The vertical anchor point of the corner
            const UiAnchor* horizontalAnchor;     ///< The horizontal anchor point of the corner
        };

        // Public interface
    public:
        /** The position of the object */
        UiPosition pos;

        /** The width of the object */
        UiSize width;
        /** The height of the object */
        UiSize height;

        /** The bounding box of the object */
        const UiFrame frame = UiFrame(pos.x, pos.y, pos.z, width, height);

        // The 4 anchors points of the object
        /** Top anchor points of the object */
        const UiAnchor top    = {0, AnchorDir::Top,    pos.y};
        /** Right anchor points of the object */
        const UiAnchor right  = {0, AnchorDir::Right,  pos.x + width};
        /** Bottom anchor points of the object */
        const UiAnchor bottom = {0, AnchorDir::Bottom, pos.y + height};
        /** Left anchor points of the object */
        const UiAnchor left   = {0, AnchorDir::Left,   pos.x};

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
        UiSize topMargin;
        /** The margin from the right anchor point */
        UiSize rightMargin;
        /** The margin from the bottom anchor point */
        UiSize bottomMargin;
        /** The margin from the left anchor point */
        UiSize leftMargin;

        /**
         * @brief Construct a new Ui Component object
         * 
         * This construct an empty ui component at coord (0.0f, 0.0f, 0.0f) of size 0, 0
         */
        UiComponent() { }
        
        /**
         * @brief Construct a new Ui Component object
         * 
         * @param frame The reference frame for this object
         * 
         * This construct an ui component from a given reference
         * The position and size of the newly created object is automatically updated on the fly
         */
        UiComponent(const UiFrame& frame) : pos(&frame.pos), width(&frame.w), height(&frame.h) { }

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

            top.id    = entityId;
            right.id  = entityId;
            bottom.id = entityId;
            left.id   = entityId;

            pos.setEntity(entityId, ecsRef);

            top.id    = this->entityId;
            right.id  = this->entityId;
            bottom.id = this->entityId;
            left.id   = this->entityId;

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
        inline void setX(const float& value)                { pos.x = value; update(); }
        inline void setY(const float& value)                { pos.y = value; update(); }
        inline void setZ(const float& value)                { pos.z = value; update(); }

        inline void setX(const UiSize& value)               { pos.x = value; update(); if(ecsRef and value.getEntityId() != 0 and value.getEntityId() != entityId) ecsRef->sendEvent(UiComponentInternalChangeEvent{value.getEntityId(), entityId});}
        inline void setY(const UiSize& value)               { pos.y = value; update(); if(ecsRef and value.getEntityId() != 0 and value.getEntityId() != entityId) ecsRef->sendEvent(UiComponentInternalChangeEvent{value.getEntityId(), entityId});}
        inline void setZ(const UiSize& value)               { pos.z = value; update(); if(ecsRef and value.getEntityId() != 0 and value.getEntityId() != entityId) ecsRef->sendEvent(UiComponentInternalChangeEvent{value.getEntityId(), entityId});}

        inline void setWidth(const float& value)            { width = value; update(); }
        inline void setHeight(const float& value)           { height = value; update(); }

        inline void setWidth(const UiSize& value)           { width = value; update(); if(ecsRef and value.getEntityId() != 0 and value.getEntityId() != entityId) ecsRef->sendEvent(UiComponentInternalChangeEvent{value.getEntityId(), entityId});}
        inline void setHeight(const UiSize& value)          { height = value; update(); if(ecsRef and value.getEntityId() != 0 and value.getEntityId() != entityId) ecsRef->sendEvent(UiComponentInternalChangeEvent{value.getEntityId(), entityId});}

        inline void setTopAnchor(const UiAnchor& anchor)    { topAnchor = &anchor; update(); if(ecsRef and anchor.id != 0 and anchor.id != entityId) ecsRef->sendEvent(UiComponentInternalChangeEvent{anchor.id, entityId});}
        inline void setRightAnchor(const UiAnchor& anchor)  { rightAnchor = &anchor; update(); if(ecsRef and anchor.id != 0 and anchor.id != entityId) ecsRef->sendEvent(UiComponentInternalChangeEvent{anchor.id, entityId});}
        inline void setBottomAnchor(const UiAnchor& anchor) { bottomAnchor = &anchor; update(); if(ecsRef and anchor.id != 0 and anchor.id != entityId) ecsRef->sendEvent(UiComponentInternalChangeEvent{anchor.id, entityId});}
        inline void setLeftAnchor(const UiAnchor& anchor)   { leftAnchor = &anchor; update(); if(ecsRef and anchor.id != 0 and anchor.id != entityId) ecsRef->sendEvent(UiComponentInternalChangeEvent{anchor.id, entityId});}

        inline void setTopMargin(const int& value)          { topMargin = value; update(); }
        inline void setRightMargin(const int& value)        { rightMargin = value; update(); }
        inline void setBottomMargin(const int& value)       { bottomMargin = value; update(); }
        inline void setLeftMargin(const int& value)         { leftMargin = value; update(); }

        inline void setTopMargin(const UiSize& value)       { topMargin = value; update(); if(ecsRef and value.getEntityId() != 0 and value.getEntityId() != entityId) ecsRef->sendEvent(UiComponentInternalChangeEvent{value.getEntityId(), entityId}); }
        inline void setRightMargin(const UiSize& value)     { rightMargin = value; update(); if(ecsRef and value.getEntityId() != 0 and value.getEntityId() != entityId) ecsRef->sendEvent(UiComponentInternalChangeEvent{value.getEntityId(), entityId});}
        inline void setBottomMargin(const UiSize& value)    { bottomMargin = value; update(); if(ecsRef and value.getEntityId() != 0 and value.getEntityId() != entityId) ecsRef->sendEvent(UiComponentInternalChangeEvent{value.getEntityId(), entityId});}
        inline void setLeftMargin(const UiSize& value)      { leftMargin = value; update(); if(ecsRef and value.getEntityId() != 0 and value.getEntityId() != entityId) ecsRef->sendEvent(UiComponentInternalChangeEvent{value.getEntityId(), entityId});}

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

    private:
        friend void serialize<>(Archive& archive, const UiComponent& value);

        // Pointer to anchors where this object is tied

        /** Pointer to the top attached anchor */
        const UiAnchor* topAnchor    = nullptr;
        /** Pointer to the right attached anchor */
        const UiAnchor* rightAnchor  = nullptr;
        /** Pointer to the bottom attached anchor */
        const UiAnchor* bottomAnchor = nullptr;
        /** Pointer to the left attached anchor */
        const UiAnchor* leftAnchor   = nullptr;

        EntitySystem* ecsRef = nullptr;

        _unique_id entityId = 0;
    };

    template <>
    void serialize(Archive& archive, const UiSize& value);

    template <>
    void serialize(Archive& archive, const AnchorDir& value);

    template <>
    void serialize(Archive& archive, const UiAnchor& value);

    template <>
    void serialize(Archive& archive, const UiPosition::UiPosValue& value);

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
                x = static_cast<UiSize>(comp.pos.x); 
                y = static_cast<UiSize>(comp.pos.y);
                z = static_cast<UiSize>(comp.pos.z);
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

            auto& vec = parentalMap[event.parent];
            
            auto it = std::find(vec.begin(), vec.end(), event.child);

            if(it == vec.end())
                vec.push_back(event.child);
        }

        virtual void onEvent(const UiSizeChangeEvent& event) override
        {
            std::unordered_map<_unique_id, std::vector<_unique_id>>::iterator it;
            std::vector<_unique_id> vec;

            {
                std::lock_guard lock(m);

                it = parentalMap.find(event.id);
                
                if(it != parentalMap.end())
                {
                    vec = it->second;
                }
            }

            // for(auto& it : parentalMap)
            // {
            //     std::cout << it.first << ": ";
            //     for(auto& it2 : it.second)
            //     {
            //         std::cout << it2 << " ";
            //     }

            //     std::cout << std::endl;
            // }
            
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