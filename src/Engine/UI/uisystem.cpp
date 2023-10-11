#include "uisystem.h"

#include "constant.h"
#include "logger.h"

#include "Renderer/renderer.h"

namespace pg
{
    namespace
	{
		static constexpr char const * DOM = "Ui System";
	}

    template <>
    UiSize operator-(const UiPosition::UiPosValue& lhs, const UiSize& rhs)
    {
        return UiSize(static_cast<UiSize>(lhs), -1.0f, rhs.value);
    }

    /**
     * @brief Specialization of the serialize function for UiSize 
     * 
     * @param archive A references to the archive
     * @param value The ui size value
     */
    template <>
    void serialize(Archive& archive, const UiSize& value)
    {
        LOG_THIS(DOM);

        archive.startSerialization("UiSize");

        serialize(archive, "value", static_cast<float>(value));

        archive.endSerialization();
    }

    /**
     * @brief Specialization of the serialize function for AnchorDir 
     * 
     * @param archive A references to the archive
     * @param value The anchor dir value
     */
    template <>
    void serialize(Archive& archive, const AnchorDir& value)
    {
        LOG_THIS(DOM);

        archive.startSerialization("AnchorDir");

        std::string anchorDirString;

        switch(value)
        {
            case AnchorDir::Top:
                anchorDirString = "Top"; break;
            case AnchorDir::Right:
                anchorDirString = "Right"; break;
            case AnchorDir::Bottom:
                anchorDirString = "Bottom"; break;
            case AnchorDir::Left:
                anchorDirString = "Left"; break;
        }

        serialize(archive, "dir", anchorDirString);

        archive.endSerialization();
    }

    /**
     * @brief Specialization of the serialize function for Anchor 
     * 
     * @param archive A references to the archive
     * @param value The anchor value
     */
    template <>
    void serialize(Archive& archive, const UiAnchor& value)
    {
        LOG_THIS(DOM);

        archive.startSerialization("UiAnchor");

        serialize(archive, "entityId", value.id);
        serialize(archive, "anchorDir", value.anchorDir);

        archive.endSerialization();
    }

    /**
     * @brief Specialization of the serialize function for UiPosValue 
     * 
     * @param archive A references to the archive
     * @param value The ui pos value
     */
    template <>
    void serialize(Archive& archive, const UiPosition::UiPosValue& value)
    {
        LOG_THIS(DOM);

        archive.startSerialization("UiPosValue");

        switch (value.type)
        {
            case UiPosition::UiPosValue::UiPosType::Anchor:
                serialize(archive, "type",   std::string("anchor"));
                serialize(archive, "anchor", value.value.anchor);
                break;

            case UiPosition::UiPosValue::UiPosType::Value:
                serialize(archive, "type",  std::string("value"));
                serialize(archive, "value", value.value.size);
                break;
        }

        archive.endSerialization();
    }

    /**
     * @brief Specialization of the serialize function for UiPosition 
     * 
     * @param archive A references to the archive
     * @param value The ui position value
     */
    template <>
    void serialize(Archive& archive, const UiPosition& value)
    {
        LOG_THIS(DOM);

        archive.startSerialization("UiPosition");

        serialize(archive, "x", value.x);
        serialize(archive, "y", value.y);
        serialize(archive, "z", value.z);

        archive.endSerialization();
    }

    /**
     * @brief Specialization of the serialize function for UiFrame 
     * 
     * @param archive A references to the archive
     * @param value The ui frame value
     */
    template <>
    void serialize(Archive& archive, const UiFrame& value)
    {
        LOG_THIS(DOM);

        archive.startSerialization("UiFrame");

        serialize(archive, "pos", value.pos);
        serialize(archive, "w", value.w);
        serialize(archive, "h", value.h);

        archive.endSerialization();
    }

    /**
     * @brief Specialization of the serialize function for UiComponent 
     * 
     * @param archive A references to the archive
     * @param value The ui component value
     */
    template <>
    void serialize(Archive& archive, const UiComponent& value)
    {
        LOG_THIS(DOM);

        // Can't actually serialize anchors here cause they are just pointers to other components
        // So they don't hold the same value each time

        archive.startSerialization("UiComponent");

        serialize(archive, "visibility",    value.isVisible());
        serialize(archive, "pos",           value.pos);
        serialize(archive, "width",         value.width);
        serialize(archive, "height",        value.height);

        serialize(archive, "topMargin",     value.topMargin);
        serialize(archive, "rightMargin",   value.rightMargin);
        serialize(archive, "bottomMargin",  value.bottomMargin);
        serialize(archive, "leftMargin",    value.leftMargin);

        UiAnchor emptyAnchor = {0, AnchorDir::Top, 0.0f};

        if(value.topAnchor == nullptr)
            serialize(archive, "topAnchor", emptyAnchor);
        else
            serialize(archive, "topAnchor", *value.topAnchor);

        if(value.leftAnchor == nullptr)
            serialize(archive, "leftAnchor", emptyAnchor);
        else
            serialize(archive, "leftAnchor", *value.leftAnchor);

        if(value.bottomAnchor == nullptr)
            serialize(archive, "bottomAnchor", emptyAnchor);
        else
            serialize(archive, "bottomAnchor", *value.bottomAnchor);

        if(value.rightAnchor == nullptr)
            serialize(archive, "rightAnchor", emptyAnchor);
        else
            serialize(archive, "rightAnchor", *value.rightAnchor);

        archive.endSerialization();
    }

    /**
     * @brief Specialization of the deserialize function for UiSize
     * 
     * @param serializedString A serialized string
     * @return UiSize An UiSize object contructed via the serialization string
     */
    template <>
    UiSize deserialize(const UnserializedObject& serializedString)
    {
        LOG_THIS(DOM);

        UiSize size;

        std::string type = "";

        if(serializedString.isNull())
            LOG_ERROR(DOM, "Element is null");
        else
        {
            size = deserialize<float>(serializedString["value"]);
        }

        return size;
    }

    /**
     * @brief Specialization of the deserialize function for UiPosition
     * 
     * @param serializedString A serialized string
     * @return UiPosition An UiPosition object contructed via the serialization string
     */
    template <>
    UiPosition deserialize(const UnserializedObject& serializedString)
    {
        LOG_THIS(DOM);

        UiPosition pos;

        std::string type = "";

        if(serializedString.isNull())
            LOG_ERROR(DOM, "Element is null");
        else
        {
            pos.x = deserialize<UiSize>(serializedString["x"]);
            pos.y = deserialize<UiSize>(serializedString["y"]);
            pos.z = deserialize<UiSize>(serializedString["z"]);
        }

        return pos;
    }

    /**
     * @brief Specialization of the deserialize function for UiFrame
     * 
     * @param serializedString A serialized string
     * @return UiFrame An UiFrame object contructed via the serialization string
     */
    template <>
    UiFrame deserialize(const UnserializedObject& serializedString)
    {
        LOG_THIS(DOM);

        UiFrame frame;

        std::string type = "";

        if(serializedString.isNull())
            LOG_ERROR(DOM, "Element is null");
        else
        {
            LOG_INFO(DOM, "Deserializing an UiFrame");

            frame.pos = deserialize<UiPosition>(serializedString["pos"]);
            frame.w   = deserialize<UiSize>(serializedString["w"]);
            frame.h   = deserialize<UiSize>(serializedString["h"]);
        }

        return frame;
    }

    /**
     * @brief Specialization of the deserialize function for UiComponent
     * 
     * @param serializedString A serialized string
     * @return UiComponent An UiComponent object contructed via the serialization string
     */
    template <>
    UiComponent deserialize(const UnserializedObject& serializedString)
    {
        LOG_THIS(DOM);

        UiComponent component;

        std::string type = "";

        if(serializedString.isNull())
            LOG_ERROR(DOM, "Element is null");
        else
        {
            LOG_INFO(DOM, "Deserializing an UiComponent");

            deserialize<bool>(serializedString["visibility"]) ? component.show() : component.hide();

            component.pos           = deserialize<UiPosition>(serializedString["pos"]);
            component.width         = deserialize<UiSize>(serializedString["width"]);
            component.height        = deserialize<UiSize>(serializedString["height"]);

            component.topMargin     = deserialize<UiSize>(serializedString["topMargin"]);
            component.rightMargin   = deserialize<UiSize>(serializedString["rightMargin"]);
            component.bottomMargin  = deserialize<UiSize>(serializedString["bottomMargin"]);
            component.leftMargin    = deserialize<UiSize>(serializedString["leftMargin"]);

            component.update();
        }

        return component;
    }

    UiComponent::UiComponent(const UiComponent& rhs)
    {
        LOG_THIS_MEMBER(DOM);

        // Copy ui component stuff

        this->visible       = rhs.visible;
        this->pos           = rhs.pos;
        this->width         = rhs.width;
        this->height        = rhs.height;

        this->topAnchor     = rhs.topAnchor;
        this->rightAnchor   = rhs.rightAnchor;
        this->bottomAnchor  = rhs.bottomAnchor;
        this->leftAnchor    = rhs.leftAnchor;

        this->topMargin     = rhs.topMargin;
        this->rightMargin   = rhs.rightMargin;
        this->bottomMargin  = rhs.bottomMargin;
        this->leftMargin    = rhs.leftMargin;

        // Copy entity stuff

        this->ecsRef        = rhs.ecsRef;
        this->entityId      = rhs.entityId;

        top.id    = this->entityId;
        right.id  = this->entityId;
        bottom.id = this->entityId;
        left.id   = this->entityId;

        pos.setEntity(entityId, ecsRef);

        width.setEntity(entityId, ecsRef);
        height.setEntity(entityId, ecsRef);

        topMargin.setEntity(entityId, ecsRef);
        leftMargin.setEntity(entityId, ecsRef);
        rightMargin.setEntity(entityId, ecsRef);
        bottomMargin.setEntity(entityId, ecsRef);
    }

    bool UiComponent::inBound(int x, int y) const
    {
        LOG_THIS_MEMBER(DOM);

        if(not visible)
            return false;

        // Lockup x and y only once
        const float xValue = static_cast<UiSize>(this->pos.x);
        const float yValue = static_cast<UiSize>(this->pos.y);

        return x > xValue && x < (xValue + this->width) && y < (yValue + this->height) && y > yValue;
    }

    bool UiComponent::inBound(const constant::Vector2D& vec2) const
    {
        LOG_THIS_MEMBER(DOM);

        return inBound(vec2.x, vec2.y);
    }

    void UiComponent::render(MasterRenderer*)
    {
        LOG_ERROR(DOM, "Called base Render of UiComponent when it should never be !");
    }

    void UiComponent::update()
    {
        LOG_THIS_MEMBER(DOM);
        
        if(topAnchor != nullptr && bottomAnchor != nullptr)
        {
            this->height = (bottomAnchor->anchorPoint - bottomMargin) - (topAnchor->anchorPoint - topMargin);
            this->pos.y = topAnchor->anchorPoint + topMargin;
        }
        else if(topAnchor != nullptr && bottomAnchor == nullptr)
        {
            this->pos.y = topAnchor->anchorPoint + topMargin;
        }
        else if(topAnchor == nullptr && bottomAnchor != nullptr)
        {
            this->pos.y = (bottomAnchor->anchorPoint - bottomMargin) - this->height;
        }

        if(rightAnchor != nullptr && leftAnchor != nullptr)
        {
            this->width = (rightAnchor->anchorPoint - rightMargin) - (leftAnchor->anchorPoint - leftMargin);
            this->pos.x = leftAnchor->anchorPoint + leftMargin;
        }
        else if(rightAnchor != nullptr && leftAnchor == nullptr)
        {
            this->pos.x = (rightAnchor->anchorPoint - rightMargin) - this->width;
        }
        else if(rightAnchor == nullptr && leftAnchor != nullptr)
        {
            this->pos.x = leftAnchor->anchorPoint + leftMargin;
        }

        if(ecsRef)
            ecsRef->sendEvent(UiComponentChangeEvent{entityId});
            // ecsRef->sendEvent(UiComponentInternalChangeEvent{});
    }
}