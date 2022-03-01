#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <algorithm>

#include <iostream>

namespace pg
{
    enum class UiOrientation
    {
        VERTICAL,
        HORIZONTAL
    };

    //TODO check if a pointer to the current uisize is not already present in the children of the operation to avoid an infinite lockup
    //TODO make the operation here constant and then change uisystem and ui animation accordingly to make everything constant
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

            typedef std::function<void()> UpdateCallback;

            struct CallbackFunctor
            {
                CallbackFunctor(const UiValue *ref, const UpdateCallback& callback) : ref(ref), callback(callback) {}

                bool operator==(const CallbackFunctor& rhs) const { return ref == rhs.ref; }

                const UiValue *ref; // Reference to the UiSize for comparaison reasons;
                UpdateCallback callback;
            };
            
        public:
            UiValue(const float& pixelSize = 0.0f, const float& scaleValue = 0.0f, std::shared_ptr<UiValue> ref1 = nullptr, std::shared_ptr<UiValue> ref2 = nullptr, const UiSizeOpType& op = UiSizeOpType::NONE) : pixelSize(pixelSize), scaleValue(scaleValue), refSize1(ref1), refSize2(ref2), opType(op)
            {
                linkCallbacksWithChildren();

                calculateCurrentValue();
            }

            ~UiValue()
            { 
                unlinkCallbacksFromChildren();
            }

            float returnCurrentSize() const { return currentValue; }

        private:
            void calculateCurrentValue()
            {
                const float refSizeValue1 = refSize1 == nullptr ? 0.0f : refSize1->returnCurrentSize();
                const float refSizeValue2 = refSize2 == nullptr ? 0.0f : refSize2->returnCurrentSize();

                const float refSize1Result = pixelSize + refSizeValue1 * scaleValue;

                //TODO make exception for division by 0
                switch(opType)
                {
                    case UiSizeOpType::ADD:
                        currentValue = refSize1Result + refSizeValue2;
                        break;
                    case UiSizeOpType::SUB:
                        currentValue = refSize1Result - refSizeValue2;
                        break;
                    case UiSizeOpType::MUL:
                        currentValue = refSize1Result * refSizeValue2;
                        break;
                    case UiSizeOpType::DIV:
                        currentValue = refSize1Result / refSizeValue2;
                        break;
                    case UiSizeOpType::NONE:
                    default:
                        currentValue = refSize1Result;

                    updateAllChildren();
                }
            }

            UpdateCallback callback = [&](){ this->calculateCurrentValue(); };
            CallbackFunctor updateCallback = {this, callback};

            void updateAllChildren() const
            {
                for(auto& callbackObject : childUpdateCallbacks)
                    callbackObject.callback();
            }

            void registerCallback(const CallbackFunctor& callback)
            {
                childUpdateCallbacks.push_back(callback);
            }

            void removeCallback(const CallbackFunctor& callback)
            {
                childUpdateCallbacks.erase(std::remove(childUpdateCallbacks.begin(), childUpdateCallbacks.end(), callback));

                // Remove all instance of the object in case it was registered multiple times
                //while(it != childUpdateCallbacks.end())
                //{
                //    childUpdateCallbacks.erase(it);
//
                //    it = std::remove(childUpdateCallbacks.begin(), childUpdateCallbacks.end(), callback);
                //}
                
            }

            void linkCallbacksWithChildren()
            {
                if(refSize1 != nullptr)
                    refSize1->registerCallback(updateCallback);

                if(refSize2 != nullptr)
                    refSize2->registerCallback(updateCallback);
            }

            void unlinkCallbacksFromChildren()
            {
                if(refSize1 != nullptr)
                    refSize1->removeCallback(updateCallback);

                if(refSize2 != nullptr)
                    refSize2->removeCallback(updateCallback);
            }

            float pixelSize = 0.0f;
            float scaleValue = 0.0f;
            std::shared_ptr<UiValue> refSize1;
            std::shared_ptr<UiValue> refSize2;
            UiSizeOpType opType = UiSizeOpType::NONE;

            float currentValue = 0.0f;

            std::vector<CallbackFunctor> childUpdateCallbacks;
        };

    public:
        UiSize(const float& pixelSize = 0.0f, const float& scaleValue = 0.0f, const std::shared_ptr<UiValue>& ref1 = nullptr, const std::shared_ptr<UiValue>& ref2 = nullptr, const UiValue::UiSizeOpType& op = UiValue::UiSizeOpType::NONE) : value(std::make_shared<UiValue>(pixelSize, scaleValue, ref1, ref2, op)) {}
        UiSize(const UiSize* size) : UiSize(0.0f, 1.0f, size->value, nullptr, UiValue::UiSizeOpType::NONE) {}
        UiSize(const UiSize& size)
        {
            if(value != nullptr)
                value->unlinkCallbacksFromChildren();

            value = size.value;
        } 

        void operator=(const UiSize& rhs) // use to copy a ui size
        {
            value->unlinkCallbacksFromChildren();

            value->pixelSize = rhs.value->pixelSize;
            value->scaleValue = rhs.value->scaleValue;
            value->refSize1 = rhs.value->refSize1;
            value->refSize2 = rhs.value->refSize2;
            value->opType = rhs.value->opType;

            value->currentValue = rhs.value->currentValue;

            value->linkCallbacksWithChildren();
            value->calculateCurrentValue();
        }

        /** Use to make this ui size refer to another */
        void operator=(const UiSize *rhs) 
        {
            value->unlinkCallbacksFromChildren();

            value->pixelSize = 0.0f;
            value->scaleValue = 1.0f;
            value->refSize1 = rhs->value;
            value->refSize2 = nullptr;
            value->opType = UiValue::UiSizeOpType::NONE;
            
            value->linkCallbacksWithChildren();
            value->calculateCurrentValue();
        }

        void operator=(const int& rhs)
        {
            value->unlinkCallbacksFromChildren();

            value->pixelSize = rhs;
            value->scaleValue = 0.0f;
            value->refSize1 = nullptr;
            value->refSize2 = nullptr;
            value->opType = UiValue::UiSizeOpType::NONE;

            value->calculateCurrentValue();
        }

        void operator=(const float& rhs)
        {
            value->unlinkCallbacksFromChildren();

            value->pixelSize = rhs;
            value->scaleValue = 0.0f;
            value->refSize1 = nullptr;
            value->refSize2 = nullptr;
            value->opType = UiValue::UiSizeOpType::NONE;

            value->calculateCurrentValue();
        }

        UiSize& operator+=(const UiSize& rhs)
        {
            //TODO: 
            /*
            UiSize current;
            current.value->pixelSize = value->pixelSize;
            current.value->scaleValue = value->scaleValue;
            current.value->refSize1 = value->refSize1;
            current.value->refSize2 = value->refSize2;
            current.value->opType = value->opType;

            value->currentValue = rhs.value->currentValue;
            
            value->childUpdateCallbacks = rhs.value->childUpdateCallbacks;

            current.value->linkCallbacksWithChildren();

            value->unlinkCallbacksFromChildren();

            value->pixelSize = 0.0f;
            value->scaleValue = 1.0f;
            value->refSize1 = current.value;
            value->refSize2 = rhs.value;
            value->opType = UiValue::UiSizeOpType::ADD;

            value->linkCallbacksWithChildren();

            value->calculateCurrentValue();

            //value->updateAllChildren();
            */
            return *this;
        }

        UiSize operator*(const float& rhs) const 
        {
            return UiSize(0.0f, rhs, this->value);
        }

        UiSize operator*(const int& rhs) const
        {
            return UiSize(0.0f, rhs, this->value);
        }

        UiSize operator*(const UiSize& rhs) const
        {
            return UiSize(0.0f, 1.0f, this->value, rhs.value, UiValue::UiSizeOpType::MUL);
        }

        //TODO make exception for division by 0
        UiSize operator/(const int& rhs) const 
        {
            return UiSize(0.0f, 1.0f / rhs, this->value);
        }

        UiSize operator/(const float& rhs) const 
        {
            return UiSize(0.0f, 1.0f / rhs, this->value);
        }

        UiSize operator/(const UiSize& rhs) const
        {
            return UiSize(0.0f, 1.0f, this->value, rhs.value, UiValue::UiSizeOpType::DIV);
        }

        UiSize operator+(const int& rhs) const
        {
            return UiSize(rhs, 1.0f, this->value);
        }

        UiSize operator+(const float& rhs) const
        {
            return UiSize(rhs, 1.0f, this->value);
        }

        UiSize operator+(const UiSize& rhs) const
        {
            return UiSize(0.0f, 1.0f, this->value, rhs.value, UiValue::UiSizeOpType::ADD);
        }

        UiSize operator-(const int& rhs) const 
        {
            return UiSize(-rhs, 1.0f, this->value);
        }

        UiSize operator-(const float& rhs) const
        {
            return UiSize(-rhs, 1.0f, this->value);
        }

        UiSize operator-(const UiSize& rhs) const 
        {
            return UiSize(0.0f, 1.0f, this->value, rhs.value, UiValue::UiSizeOpType::SUB);
        }

        UiSize operator-() const
        {
            return UiSize(0.0f, -1.0f, this->value);
        }

        template<typename Type>
        friend UiSize operator+(const Type& lhs, const UiSize& rhs);

        template<typename Type>
        friend UiSize operator-(const Type& lhs, const UiSize& rhs);

        template<typename Type>
        friend UiSize operator*(const Type& lhs, const UiSize& rhs);

        template<typename Type>
        friend UiSize operator/(const Type& lhs, const UiSize& rhs);

        operator float() const
        {
            return value->returnCurrentSize();
        }

    private:
        std::shared_ptr<UiValue> value;
    };

    template<typename Type>
    UiSize operator+(const Type& lhs, const UiSize& rhs)
    {
        return rhs + lhs;
    }

    template<typename Type>
    UiSize operator-(const Type& lhs, const UiSize& rhs)
    {
        return UiSize(lhs, -1.0f, rhs.value);
    }

    template<typename Type>
    UiSize operator*(const Type& lhs, const UiSize& rhs)
    {
        return rhs * lhs;
    }

    template<typename Type>
    UiSize operator/(const Type& lhs, const UiSize& rhs)
    {
        return UiSize(lhs, 0.0f, nullptr, rhs.value, UiSize::UiValue::UiSizeOpType::DIV);
    }

    struct UiPosition 
    {
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

        UiSize x;
        UiSize y;
        UiSize z;
    };

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