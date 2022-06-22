#include "uisystem.h"

#include "constant.h"
#include "logger.h"
#include "serialization.h"

#include "Renderer/renderer.h"

namespace pg
{
    namespace
	{
		const char * DOM = "Ui System";
	}

    template<>
    void serialize(Archive& archive, const UiSize& value)
    {
        LOG_THIS(DOM);

        archive.startSerialization("UiSize");

        serialize(archive, "value", static_cast<float>(value));

        archive.endSerialization();
    }

    template<>
    void serialize(Archive& archive, const UiPosition& value)
    {
        LOG_THIS(DOM);

        archive.startSerialization("UiPosition");

        serialize(archive, "x", value.x);
        serialize(archive, "y", value.y);
        serialize(archive, "z", value.z);

        archive.endSerialization();
    }

    template<>
    void serialize(Archive& archive, const UiFrame& value)
    {
        LOG_THIS(DOM);

        archive.startSerialization("UiFrame");

        serialize(archive, "pos", value.pos);
        serialize(archive, "w", value.w);
        serialize(archive, "h", value.h);

        archive.endSerialization();
    }

    template<>
    void serialize(Archive& archive, const UiComponent& value)
    {
        LOG_THIS(DOM);

        archive.startSerialization("UiComponent");

        serialize(archive, "visibility", value.isVisible());
        serialize(archive, "pos", value.pos);
        serialize(archive, "width", value.width);
        serialize(archive, "height", value.height);

        serialize(archive, "topMargin", value.topMargin);
        serialize(archive, "rightMargin", value.rightMargin);
        serialize(archive, "bottomMargin", value.bottomMargin);
        serialize(archive, "leftMargin", value.leftMargin);

        archive.endSerialization();
    }

    template<>
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

    template<>
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

    template<>
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
            frame.w = deserialize<UiSize>(serializedString["w"]);
            frame.h = deserialize<UiSize>(serializedString["h"]);
        }

        return frame;
    }

    template<>
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

            component.pos = deserialize<UiPosition>(serializedString["pos"]);
            component.width = deserialize<UiSize>(serializedString["width"]);
            component.height = deserialize<UiSize>(serializedString["height"]);

            component.topMargin = deserialize<UiSize>(serializedString["topMargin"]);
            component.rightMargin = deserialize<UiSize>(serializedString["rightMargin"]);
            component.bottomMargin = deserialize<UiSize>(serializedString["bottomMargin"]);
            component.leftMargin = deserialize<UiSize>(serializedString["leftMargin"]);

            component.update();
        }

        return component;
    }

    UiComponent::UiComponent(const UiComponent& rhs)
    {
        //TODO remove the previous reference of rhs inside the parent and push this pointer inside the parent child list to avoid resize error when this is being copied
        //also don t forget to call this constructor when creating the copy constructor of the child class
        this->visible = rhs.visible;
        this->pos = rhs.pos;
        this->width = rhs.width;
        this->height = rhs.height;
        this->topAnchor = rhs.topAnchor;
        this->rightAnchor = rhs.rightAnchor;
        this->bottomAnchor = rhs.bottomAnchor;
        this->leftAnchor = rhs.leftAnchor;

        this->topMargin = rhs.topMargin;
        this->rightMargin = rhs.rightMargin;
        this->bottomMargin = rhs.bottomMargin;
        this->leftMargin = rhs.leftMargin;
    }

    bool UiComponent::inBound(int x, int y) const
    {
        // Lockup x and y only once
        const float xValue = x;
        const float yValue = y;

        return xValue > this->pos.x && xValue < (this->pos.x + this->width) && yValue < (this->pos.y + this->height) && yValue > this->pos.y;
    }

    bool UiComponent::inBound(const constant::Vector2D& vec2) const
    {
        return inBound(vec2.x, vec2.y);
    }

    void UiComponent::render(MasterRenderer*)
    {
        LOG_ERROR(DOM, "Called Render of UiComponent when it should never be !");
    }

    void UiComponent::update()
    {
        if(topAnchor != nullptr && bottomAnchor != nullptr)
        {
            this->height = (*bottomAnchor - bottomMargin) - (*topAnchor - topMargin);
            //this->height = UiSize(0.0f, 1.0f, new UiSize(-bottomMargin, 1.0f, bottomAnchor), new UiSize(-topMargin, 1.0f, topAnchor), UiSize::UiSizeOpType::SUB); // todo change this because () create elements that are temporary
            this->pos.y = *topAnchor + topMargin;
        }
        else if(topAnchor != nullptr && bottomAnchor == nullptr)
        {
            this->pos.y = *topAnchor + topMargin;
        }
        else if(topAnchor == nullptr && bottomAnchor != nullptr)
        {
            //this->pos.y = (*bottomAnchor - bottomMargin) - this->height;
            this->pos.y = (*bottomAnchor - bottomMargin) - this->height;
            //this->pos.y = UiSize(-this->height, 1.0f, new UiSize(-bottomMargin, 1.0f, bottomAnchor));
        }

        if(rightAnchor != nullptr && leftAnchor != nullptr)
        {
            this->width = (*rightAnchor - rightMargin) - (*leftAnchor - leftMargin);
            //this->width = UiSize(0.0f, 1.0f, new UiSize(-rightMargin, -1.0f, rightAnchor), new UiSize(-leftMargin, 1.0f, leftAnchor), UiSize::UiSizeOpType::SUB);
            this->pos.x = *leftAnchor + leftMargin;
        }
        else if(rightAnchor != nullptr && leftAnchor == nullptr)
        {
            this->pos.x = (*rightAnchor - rightMargin) - this->width;
            //this->pos.x = UiSize(this->width, 1.0f, new UiSize(-rightMargin, 1.0f, rightAnchor));
        }
        else if(rightAnchor == nullptr && leftAnchor != nullptr)
        {
            this->pos.x = *leftAnchor + leftMargin;
        }
    }
}