#pragma once

namespace pg
{
    //TODO make the operation here constant and then change uisystem and ui animation accordingly to make everything constant
    class UiSize
    {
    public:
        enum class UiSizeOpType
        {
            ADD,
            SUB,
            MUL,
            DIV,

            NONE
        };
        
        UiSize(const float& pixelSize = 0, const float& scaleValue = 0, const UiSize* ref1 = nullptr, const UiSize* ref2 = nullptr,  const UiSizeOpType& op = UiSizeOpType::NONE) : pixelSize(pixelSize), scaleValue(scaleValue), refSize1(ref1), refSize2(ref2), opType(op) {}
        UiSize(const UiSize& size) : UiSize(size.pixelSize, size.scaleValue, size.refSize1, size.refSize2, size.opType) {} // TODO create a copy contruct with a bool to delete pointer
        UiSize(const UiSize* size) : UiSize(0.0f, 1.0f, size, nullptr, UiSizeOpType::NONE) {}

        void operator=(const UiSize& rhs) // use to copy a ui size
        {
            this->pixelSize = rhs.pixelSize;
            this->scaleValue = rhs.scaleValue;
            this->refSize1 = rhs.refSize1;
            this->refSize2 = rhs.refSize2;
            this->opType = rhs.opType;
        }

        void operator=(const UiSize *rhs) // use to make this ui size refer to another
        {
            this->pixelSize = 0;
            this->scaleValue = 1.0f;
            this->refSize1 = rhs;
            this->refSize2 = nullptr;
            this->opType = UiSizeOpType::NONE;
        }

        void operator=(const int& rhs)
        {
            this->pixelSize = rhs;
            this->scaleValue = 1.0f;
            this->refSize1 = nullptr;
            this->refSize2 = nullptr;
            this->opType = UiSizeOpType::NONE;
        }

        void operator=(const float& rhs)
        {
            this->pixelSize = rhs;
            this->scaleValue = 1.0f;
            this->refSize1 = nullptr;
            this->refSize2 = nullptr;
            this->opType = UiSizeOpType::NONE;
        }

        UiSize operator*(const float& rhs) const
        {
            return UiSize(0.0f, rhs, this);
        }

        UiSize operator*(const int& rhs) const
        {
            return UiSize(0.0f, rhs, this);
        }

        UiSize operator+(const int& rhs) const
        {
            return UiSize(rhs, 1.0f, this);
        }

        UiSize operator+(const float& rhs) const
        {
            return UiSize(rhs, 1.0f, this);
        }

        UiSize operator+(const UiSize& rhs) const
        {
            return UiSize(0.0f, 1.0f, this, &rhs, UiSizeOpType::ADD);
        }

        UiSize operator-(const int& rhs) const
        {
            return UiSize(-rhs, 1.0f, this);
        }

        UiSize operator-(const float& rhs) const
        {
            return UiSize(-rhs, 1.0f, this);
        }

        UiSize operator-(const UiSize& rhs) const
        {
            return UiSize(0.0f, 1.0f, this, &rhs, UiSizeOpType::SUB);
        }

        UiSize operator-() const
        {
            return UiSize(0.0f, -1.0f, this);
        }

        template<typename Type>
        friend Type operator+(const Type& lhs, const UiSize& rhs);

        template<typename Type>
        friend Type operator-(const Type& lhs, const UiSize& rhs);

        operator float() const
        {
            return returnCurrentSize();
        }

    private:
        float returnCurrentSize() const
        {
            const float refSizeValue1 = refSize1 == nullptr ? 0.0f : refSize1->returnCurrentSize();
            const float refSizeValue2 = refSize2 == nullptr ? 0.0f : refSize2->returnCurrentSize();

            const float refSize1Result = pixelSize + refSizeValue1 * scaleValue;

            switch(opType)
            {
                case UiSizeOpType::ADD:
                    return refSize1Result + refSizeValue2;
                    break;
                case UiSizeOpType::SUB:
                    return refSize1Result - refSizeValue2;
                    break;
                case UiSizeOpType::NONE:
                default:
                    return refSize1Result;
            }
        }

        float pixelSize = 0.0f;
        float scaleValue = 0.0f;
        const UiSize *refSize1 = nullptr;
        const UiSize* refSize2 = nullptr;
        UiSizeOpType opType = UiSizeOpType::NONE;
    };

    template<typename Type>
    Type operator+(const Type& lhs, const UiSize& rhs)
    {
        return lhs + static_cast<float>(rhs);
    }

    template<typename Type>
    Type operator-(const Type& lhs, const UiSize& rhs)
    {
        return lhs - static_cast<float>(rhs);
    }

    struct UiPosition 
    {
        UiPosition() {}
        UiPosition(const UiSize& x, const UiSize& y, const UiSize& z) { this->x = &x; this->y = &y; this->z = &z; }
        UiPosition(const UiPosition& pos) : x(pos.x), y(pos.y), z(pos.z) { }
        UiPosition(UiPosition *pos) : x(&pos->x), y(&pos->y), z(&pos->z) { }

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

        UiSize x = UiSize(0, 0, nullptr);
        UiSize y = UiSize(0, 0, nullptr);
        UiSize z = UiSize(0, 0, nullptr);    
    };

    struct UiFrame
    {
        UiFrame() {}
        UiFrame(const UiSize& x, const UiSize& y, const UiSize& z, const UiSize& w, const UiSize& h) { this->pos.x = &x; this->pos.y = &y; this->pos.z = &z; this->w = &w; this->h = &h; }
        UiFrame(const UiPosition& pos, const UiSize& w, const UiSize& h) : pos(pos), w(w), h(h) { }
        UiFrame(const UiFrame& frame) : pos(frame.pos), w(frame.w), h(frame.h) { }
        UiFrame(UiFrame *frame) : pos(&frame->pos), w(&frame->w), h(&frame->h) { }

        void operator=(const UiFrame& rhs)
        {
            pos.x = rhs.pos.x;
            pos.y = rhs.pos.y; 
            pos.z = rhs.pos.z;
            w = rhs.w;
            h = rhs.h; 
        }

        void operator=(UiFrame *rhs)
        {
            pos.x = &rhs->pos.x;
            pos.y = &rhs->pos.y; 
            pos.z = &rhs->pos.z;
            w = &rhs->w;
            h = &rhs->h; 
        }

        UiPosition pos;
        UiSize w = UiSize(0, 0, nullptr);
        UiSize h = UiSize(0, 0, nullptr);
    };
}