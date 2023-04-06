#pragma once

#include <memory>

#include "uniqueid.h"
#include "logger.h"

namespace pg
{
    // namespace
    // {
    //     const char * DOM = "Ui constant";
    // }

    enum class UiOrientation
    {
        VERTICAL,
        HORIZONTAL
    };

    //TODO check if a pointer to the current uisize is not already present in the children of the operation to avoid an infinite lockup
    //TODO make the operation here constant and then change uisystem and ui animation accordingly to make everything constant
    //TODO throw if we try to divide by zero
    class UiSize
    {
        class UiValue
        {
            friend class UiSize;

            enum class UiSizeOpType
            {
                ADD,
                SUB,
                MUL,
                DIV,

                NONE
            };
            
        public:
            UiValue(const float& pixelSize = 0.0f, const float& scaleValue = 0.0f, std::shared_ptr<UiValue> ref1 = nullptr, std::shared_ptr<UiValue> ref2 = nullptr, const UiSizeOpType& op = UiSizeOpType::NONE) : pixelSize(pixelSize), scaleValue(scaleValue), refSize1(ref1), refSize2(ref2), opType(op) { }

            float returnCurrentSize() const
            {
                // TODO
                // LOG_THIS_MEMBER(DOM);

                const float refSizeValue1 = refSize1 == nullptr ? 0.0f : refSize1->returnCurrentSize();
                const float refSizeValue2 = refSize2 == nullptr ? 0.0f : refSize2->returnCurrentSize();

                const float refSize1Result = pixelSize + refSizeValue1 * scaleValue;

                //TODO make exception for division by 0
                switch(opType)
                {
                    case UiSizeOpType::ADD:
                        return refSize1Result + refSizeValue2;
                        break;
                    case UiSizeOpType::SUB:
                        return refSize1Result - refSizeValue2;
                        break;
                    case UiSizeOpType::MUL:
                        return refSize1Result * refSizeValue2;
                        break;
                    case UiSizeOpType::DIV:
                        if(refSizeValue2 == 0.0f)
                        {
                            // TODO
                            LOG_ERROR("Ui Constant", "Division by zero");
                            return 0.0f;
                        }
                        return refSize1Result / refSizeValue2;
                        break;
                    case UiSizeOpType::NONE:
                    default:
                        return refSize1Result;
                }
            }

        private:
            float pixelSize = 0.0f;
            float scaleValue = 0.0f;
            std::shared_ptr<UiValue> refSize1;
            std::shared_ptr<UiValue> refSize2;
            UiSizeOpType opType = UiSizeOpType::NONE;
        };

    // Todo on op =  trigger an event to send an update event 
    public:
        UiSize(float pixelSize = 0.0f, float scaleValue = 0.0f, std::shared_ptr<UiValue> ref1 = nullptr, std::shared_ptr<UiValue> ref2 = nullptr, const UiValue::UiSizeOpType& op = UiValue::UiSizeOpType::NONE) : value(std::make_shared<UiValue>(pixelSize, scaleValue, ref1, ref2, op)) {}
        //UiSize(const float& pixelSize = 0.0f, const float& scaleValue = 0.0f, std::shared_ptr<UiValue> ref1 = nullptr, std::shared_ptr<UiValue> ref2 = nullptr, const UiValue::UiSizeOpType& op = UiValue::UiSizeOpType::NONE) : value(std::make_shared<UiValue>(pixelSize, scaleValue, ref1, ref2, op)) {}
        UiSize(const UiSize& size) : value(size.value) {} // TODO create a copy contruct with a bool to delete pointer
        UiSize(const UiSize* size) : UiSize(0.0f, 1.0f, size->value, nullptr, UiValue::UiSizeOpType::NONE) {}

        void operator=(const UiSize& rhs) // use to copy a ui size
        {
            //value = rhs.value;
            //value = std::make_shared<UiValue>(rhs.value->pixelSize, rhs.value->scaleValue, rhs.value->refSize1, rhs.value->refSize2, rhs.value->opType);
            value->pixelSize = rhs.value->pixelSize;
            value->scaleValue = rhs.value->scaleValue;
            value->refSize1 = rhs.value->refSize1;
            value->refSize2 = rhs.value->refSize2;
            value->opType = rhs.value->opType;
        }

        void operator=(const UiSize *rhs) // use to make this ui size refer to another
        {
            //value = std::shared_ptr<UiValue>(new UiValue(0.0f, 1.0f, rhs->value, nullptr, UiValue::UiSizeOpType::NONE));
            //value = std::make_shared<UiValue>(0.0f, 1.0f, rhs->value, nullptr, UiValue::UiSizeOpType::NONE);
            value->pixelSize = 0.0f;
            value->scaleValue = 1.0f;
            value->refSize1 = rhs->value;
            value->refSize2 = nullptr;
            value->opType = UiValue::UiSizeOpType::NONE;
        }

        void operator=(const int& rhs)
        {
            //value = std::make_shared<UiValue>(rhs, 1.0f, nullptr, nullptr, UiValue::UiSizeOpType::NONE);
            value->pixelSize = rhs;
            value->scaleValue = 0.0f;
            value->refSize1 = nullptr;
            value->refSize2 = nullptr;
            value->opType = UiValue::UiSizeOpType::NONE;
        }

        void operator=(const float& rhs)
        {
            //value = std::make_shared<UiValue>(rhs, 1.0f, nullptr, nullptr, UiValue::UiSizeOpType::NONE);
            value->pixelSize = rhs;
            value->scaleValue = 0.0f;
            value->refSize1 = nullptr;
            value->refSize2 = nullptr;
            value->opType = UiValue::UiSizeOpType::NONE;
        }

        UiSize& operator+=(const UiSize& rhs)
        {
            UiSize current;
            current.value->pixelSize = value->pixelSize;
            current.value->scaleValue = value->scaleValue;
            current.value->refSize1 = value->refSize1;
            current.value->refSize2 = value->refSize2;
            current.value->opType = value->opType;

            value->pixelSize = 0.0f;
            value->scaleValue = 1.0f;
            value->refSize1 = current.value;
            value->refSize2 = rhs.value;
            value->opType = UiValue::UiSizeOpType::ADD;

            return *this;
        }

        UiSize operator*(const float& rhs) const 
        {
            return UiSize(0.0f, rhs, this->value);
            //auto res = std::make_shared<UiSize>(0.0f, rhs, this->value);
            //return *res;
        }

        UiSize operator*(const int& rhs) const
        {
            return UiSize(0.0f, rhs, this->value);
            //auto res = std::make_shared<UiSize>(0.0f, rhs, this);
            //return *res;
        }

        UiSize operator*(const UiSize& rhs) const
        {
            return UiSize(0.0f, 1.0f, this->value, rhs.value, UiValue::UiSizeOpType::MUL);
            //auto res = std::make_shared<UiSize>(0.0f, 1.0f, this, &rhs, UiValue::UiSizeOpType::MUL);
            //return *res;
        }

        //TODO make exception for division by 0
        UiSize operator/(const int& rhs) const 
        {
            return UiSize(0.0f, 1.0f / rhs, this->value);
            //auto res = std::make_shared<UiSize>(0.0f, 1.0f / rhs, this);
            //return *res;
        }

        UiSize operator/(const float& rhs) const 
        {
            return UiSize(0.0f, 1.0f / rhs, this->value);
            //auto res = std::make_shared<UiSize>(0.0f, 1.0f / rhs, this);
            //return *res;
        }

        UiSize operator/(const UiSize& rhs) const
        {
            return UiSize(0.0f, 1.0f, this->value, rhs.value, UiValue::UiSizeOpType::DIV);
            //auto res = std::make_shared<UiSize>(0.0f, 1.0f, this, &rhs, UiValue::UiSizeOpType::DIV);
            //return *res;
        }

        UiSize operator+(const int& rhs) const
        {
            return UiSize(rhs, 1.0f, this->value);
            //auto res = std::make_shared<UiSize>(rhs, 1.0f, this);
            //return *res;
        }

        UiSize operator+(const float& rhs) const
        {
            return UiSize(rhs, 1.0f, this->value);
            //auto res = std::make_shared<UiSize>(rhs, 1.0f, this);
            //return *res;
        }

        UiSize operator+(const UiSize& rhs) const
        {
            return UiSize(0.0f, 1.0f, this->value, rhs.value, UiValue::UiSizeOpType::ADD);
            //auto res = std::make_shared<UiSize>(0.0f, 1.0f, this, &rhs, UiValue::UiSizeOpType::ADD);
            //return *res;
        }

        UiSize operator-(const int& rhs) const 
        {
            return UiSize(-rhs, 1.0f, this->value);
            //auto res = std::make_shared<UiSize>(-rhs, 1.0f, this);
            //return *res;
        }

        UiSize operator-(const float& rhs) const
        {
            return UiSize(-rhs, 1.0f, this->value);
            //auto res = std::make_shared<UiSize>(-rhs, 1.0f, this);
            //return *res;
        }

        UiSize operator-(const UiSize& rhs) const 
        {
            return UiSize(0.0f, 1.0f, this->value, rhs.value, UiValue::UiSizeOpType::SUB);
            //auto res = std::make_shared<UiSize>(0.0f, 1.0f, this, &rhs, UiValue::UiSizeOpType::SUB);
            //return *res;
        }

        UiSize operator-() const
        {
            return UiSize(0.0f, -1.0f, this->value);
            //auto res = std::make_shared<UiSize>(0.0f, -1.0f, this);
            //return *res;
        }

        template <typename Type>
        friend UiSize operator+(const Type& lhs, const UiSize& rhs);

        template <typename Type>
        friend UiSize operator-(const Type& lhs, const UiSize& rhs);

        template <typename Type>
        friend UiSize operator*(const Type& lhs, const UiSize& rhs);

        template <typename Type>
        friend UiSize operator/(const Type& lhs, const UiSize& rhs);

        operator float() const
        {
            return value->returnCurrentSize();
        }

    private:
        std::shared_ptr<UiValue> value;
    };

    template <typename Type>
    UiSize operator+(const Type& lhs, const UiSize& rhs)
    {
        return rhs + lhs;
    }

    template <typename Type>
    UiSize operator-(const Type& lhs, const UiSize& rhs)
    {
        return UiSize(lhs, -1.0f, rhs.value);
    }

    template <typename Type>
    UiSize operator*(const Type& lhs, const UiSize& rhs)
    {
        return rhs * lhs;
    }

    template <typename Type>
    UiSize operator/(const Type& lhs, const UiSize& rhs)
    {
        return UiSize(lhs, 0.0f, nullptr, rhs.value, UiSize::UiValue::UiSizeOpType::DIV);
    }

    enum class AnchorDir : uint8_t
    {
        Top = 0,
        Right,
        Bottom,
        Left
    };

    struct UiAnchor
    {
        mutable _unique_id id = 0; ///< Unique identifier of the entity oh this anchor
        AnchorDir anchorDir;       ///< Direction of the anchor point mainly for serialization purposes
        UiSize anchorPoint;        ///< The anchor point of the corner

        // UiAnchor() : id(0), anchorDir(AnchorDir::Top), anchorPoint(0.0f) {}
        // UiAnchor(_unique_id id, const AnchorDir& dir, const UiSize& size) : id(id), anchorDir(dir), anchorPoint(size) {}
    };

    struct UiPosition 
    {
        // UiPosValue always hold a reference to UiPoint assigned to it !
        struct UiPosValue
        {
            enum class UiPosType : uint8_t
            {
                Anchor,
                Value,
                UiPosValue
            };

            union UiPosV
            {
                UiPosV() { new(&size) UiSize(0.0f); }
                ~UiPosV() {}

                UiAnchor anchor;
                UiSize size;
                const UiPosValue *ref;
            };

            UiPosValue() : type(UiPosType::Value) { }

            explicit UiPosValue(const UiSize& size) : type(UiPosType::Value) { new(&value.size) UiSize(size); this->size = value.size; }
            explicit UiPosValue(const UiSize* size) : type(UiPosType::Value) { new(&value.size) UiSize(size); this->size = value.size; }
            explicit UiPosValue(const UiAnchor& anchor) : type(UiPosType::Anchor) { new(&value.anchor) UiAnchor(anchor); this->size = value.anchor.anchorPoint; }

            explicit UiPosValue(const UiPosValue& rhs)
            { 
                type = rhs.type;

                if (type == UiPosType::Anchor)
                {
                    new(&value.anchor) UiAnchor(rhs.value.anchor);
                    this->size = &value.anchor.anchorPoint;
                }
                else if (type == UiPosType::Value)
                {
                    new(&value.size) UiSize(rhs.value.size);
                    this->size = value.size;
                }
                else
                {
                    value.ref = rhs.value.ref;
                    this->size = rhs.size;
                }
            }

            explicit UiPosValue(const UiPosValue* rhs) : type(UiPosType::UiPosValue) { value.ref = rhs; size = &rhs->size; }

            ~UiPosValue() { clearOldType(); }

            void operator=(const UiPosValue& rhs)
            {
                clearOldType();

                type = rhs.type;

                if (type == UiPosType::Anchor)
                {
                    new(&value.anchor) UiAnchor(rhs.value.anchor);
                    this->size = &value.anchor.anchorPoint;
                }
                else if (type == UiPosType::Value)
                {
                    new(&value.size) UiSize(rhs.value.size);
                    this->size = value.size;
                }
                else
                {
                    value.ref = rhs.value.ref;
                    this->size = rhs.size;
                }
            }

            void operator=(const UiPosValue* rhs)
            {
                clearOldType();

                type = UiPosType::UiPosValue;

                value.ref = rhs;
                this->size = &rhs->size;
            }

            void operator=(const UiAnchor& anchor)
            {
                clearOldType();

               type = UiPosType::Anchor;
               new(&value.anchor) UiAnchor(anchor);

               this->size = &value.anchor.anchorPoint;
            }

            void operator=(const UiSize& size)
            {
                clearOldType();

                type = UiPosType::Value;
                new(&value.size) UiSize(size);

                this->size = value.size;
            }

            void operator=(const UiSize* size)
            {
                clearOldType();

                type = UiPosType::Value;
                new(&value.size) UiSize(size);

                this->size = value.size;
            }

            void operator=(float v)
            {
                clearOldType();

                type = UiPosType::Value;
                new(&value.size) UiSize(v);

                size = value.size;
            }
            
            void operator=(int v)
            {
                clearOldType();

                type = UiPosType::Value;
                new(&value.size) UiSize(v);

                size = value.size;
            }

            inline bool operator==(const UiPosValue& rhs) const
            {
                return size == rhs.size;
            }

            inline bool operator==(float rhs) const
            {
                return size == rhs;
            }

            bool operator<(const UiPosValue& rhs) const
            {
                return size < rhs.size;
            }

            bool operator>(const UiPosValue& rhs) const
            {
                return size > rhs.size;
            }

            bool operator<(int rhs) const
            {
                return size < rhs;
            }

            bool operator>(int rhs) const
            {
                return size > rhs;
            }

            void clearOldType()
            {
                if (type == UiPosType::Anchor)
                    value.anchor.~UiAnchor();
                else if (type == UiPosType::Value)
                    value.size.~UiSize();
            }

            operator UiSize() const
            {
                return size;
            }

            UiPosType type;
            UiPosV value;
            UiSize size;
        };

        UiPosition() {}
        UiPosition(const UiSize& x, const UiSize& y, const UiSize& z) { this->x = &x; this->y = &y; this->z = &z; }
        UiPosition(const UiPosition& pos) : x(pos.x), y(pos.y), z(pos.z) { }
        UiPosition(const UiPosition *pos) : x(&pos->x), y(&pos->y), z(&pos->z) { }

        void operator=(const UiPosition& rhs)
        {
            x = rhs.x;
            y = rhs.y; 
            z = rhs.z; 
        }

        void operator=(UiPosition *rhs)
        {
            x = &rhs->x;
            y = &rhs->y; 
            z = &rhs->z; 
        }

        UiPosition operator+(const UiPosition& rhs) const {
            UiPosition pos;
            pos.x = x + rhs.x;
            pos.y = y + rhs.y;
            pos.z = z + rhs.z;

            return pos;
        } 

        UiPosValue x;
        UiPosValue y;
        UiPosValue z;
    };

    template <>
    UiSize operator-(const UiPosition::UiPosValue& lhs, const UiSize& rhs);

    struct UiFrame
    {
        UiFrame() {}
        UiFrame(const UiSize& x, const UiSize& y, const UiSize& z, const UiSize& w, const UiSize& h) { this->pos.x = &x; this->pos.y = &y; this->pos.z = &z; this->w = &w; this->h = &h; }
        UiFrame(const UiPosition& pos, const UiSize& w, const UiSize& h) : pos(&pos), w(&w), h(&h) { }
        UiFrame(const UiFrame& frame) : pos(frame.pos), w(frame.w), h(frame.h) { }
        UiFrame(const UiFrame *frame) : pos(&frame->pos), w(&frame->w), h(&frame->h) { }

        void operator=(const UiFrame& rhs)
        {
            pos.x = rhs.pos.x;
            pos.y = rhs.pos.y; 
            pos.z = rhs.pos.z;
            w = rhs.w;
            h = rhs.h; 
        }

        void operator=(const UiFrame *rhs)
        {
            pos.x = &rhs->pos.x;
            pos.y = &rhs->pos.y; 
            pos.z = &rhs->pos.z;
            w = &rhs->w;
            h = &rhs->h; 
        }

        UiPosition pos;
        UiSize w;
        UiSize h;
    };
}