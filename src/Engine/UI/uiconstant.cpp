#include "uiconstant.h"

#include "uisystem.h"

namespace pg
{
    float UiSize::UiValue::returnCurrentSize() const
    {
        // TODO
        // LOG_THIS_MEMBER(DOM);

        const float refSizeValue1 = refSize1 == nullptr ? 0.0f : refSize1->returnCurrentSize();
        const float refSizeValue2 = refSize2 == nullptr ? 0.0f : refSize2->returnCurrentSize();

        const float refSize1Result = pixelSize + refSizeValue1 * scaleValue;

        float currentValue = 0.0f;

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
                if(refSizeValue2 == 0.0f)
                {
                    // TODO
                    LOG_ERROR("Ui Constant", "Division by zero");
                    currentValue = 0.0f;
                }
                currentValue = refSize1Result / refSizeValue2;
                break;
            case UiSizeOpType::NONE:
            default:
                currentValue = refSize1Result;
        }

        if(ecsRef and currentValue != oldValue)
        {
            auto entity = ecsRef->getEntity(entityId);

            if(entity and entity->has<UiComponent>())
            {
                oldValue = currentValue;
                ecsRef->sendEvent(UiSizeChangeEvent{entityId});
            }
        }

        return currentValue;
    }
}