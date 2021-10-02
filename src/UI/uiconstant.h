#pragma once

//TODO make the operation here constant and then change uisystem and ui animation accordingly to make everything constant
class UiSize
{
public:
    UiSize() {}
    UiSize(const float& pixelSize = 0, const float& scaleValue = 0, UiSize* ref = nullptr) : pixelSize(pixelSize), scaleValue(scaleValue), refSize(ref) {}
    UiSize(const UiSize& size) : pixelSize(size.pixelSize), scaleValue(size.scaleValue), refSize(size.refSize) {}

    static float returnCurrentSize(const UiSize* size)
    {
        if(size == nullptr)
            return 0;
        else
            return size->pixelSize + returnCurrentSize(size->refSize) * size->scaleValue;
    }

    void operator=(const UiSize& rhs) // use to copy a ui size
    {
        this->pixelSize = rhs.pixelSize;
        this->scaleValue = rhs.scaleValue;
        this->refSize = rhs.refSize;
    }

    void operator=(UiSize *rhs) // use to make this ui size refer to another
    {
        this->pixelSize = 0;
        this->scaleValue = 1.0f;
        this->refSize = rhs;
    }

    void operator=(const int& rhs)
    {
        this->pixelSize = rhs;
        this->scaleValue = 1.0f;
        this->refSize = nullptr;
    }

    void operator=(const float& rhs)
    {
        this->pixelSize = rhs;
        this->scaleValue = 1.0f;
        this->refSize = nullptr;
    }

    UiSize operator*(const float& rhs)
    {
        return UiSize(0.0f, rhs, this);
    }

    UiSize operator+(const int& rhs)
    {
        return UiSize(rhs, 1.0f, this);
    }

    UiSize operator+(const float& rhs)
    {
        return UiSize(rhs, 1.0f, this);
    }

    //TODO find a way to combine both uisize
    UiSize operator+(UiSize& rhs)
    {
        return UiSize(static_cast<float>(rhs), 1.0f, this);
        //return UiSize(rhs, 1.0f, this);
    }

    UiSize operator-(const int& rhs)
    {
        return UiSize(-rhs, 1.0f, this);
    }

    UiSize operator-(const float& rhs)
    {
        return UiSize(-rhs, 1.0f, this);
    }

    //TODO find a way to combine both uisize
    UiSize operator-(UiSize& rhs)
    {
        return UiSize(-rhs, 1.0f, this);
    }

    float operator-()
    {
        return -UiSize::returnCurrentSize(this);
    }

    template<typename Type>
    friend Type operator+(const Type& lhs, const UiSize& rhs);

    template<typename Type>
    friend Type operator-(const Type& lhs, const UiSize& rhs);

    //TODO check if it is okey that UiSize can t be negative and if so make it a clear condition
    operator float()
    {
        return UiSize::returnCurrentSize(this); // < 0 ? 0 : UiSize::returnCurrentSize(this);
    }
private:
    float pixelSize = 0;
    float scaleValue = 0.0f;
    UiSize *refSize;
};

template<typename Type>
Type operator+(const Type& lhs, const UiSize& rhs)
{
    return lhs + UiSize::returnCurrentSize(&rhs);
}

template<typename Type>
Type operator-(const Type& lhs, const UiSize& rhs)
{
    return lhs - UiSize::returnCurrentSize(&rhs);
}

//TODO create ctor dtor copy
struct UiPosition 
{
    UiPosition() {}
    UiPosition(UiSize& x, UiSize& y, UiSize& z) { this->x = &x; this->y = &y; this->z = &z; } // todo create a const copy operator for uisize
    UiPosition(const UiPosition& pos) : x(pos.x), y(pos.y), z(pos.z) { } // todo create a const copy operator for uisize

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