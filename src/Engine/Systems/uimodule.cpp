#include "uimodule.h"

namespace pg
{
    std::vector<SysListElement> uiElementFunctionsList(const Function *caller, EntitySystem *ecsRef, const CompRef<UiComponent>& comp)
    {
        return {{"id", comp.entityId},
                {"x", static_cast<UiSize>(comp->pos.x)}, // Todo change with a getter
                {"y", static_cast<UiSize>(comp->pos.y)}, // Todo change with a getter
                {"w", comp->width},                      // Todo change with a getter
                {"h", comp->height},                     // Todo change with a getter
                {"setX", makeFun<SetX>(caller, "setX", ecsRef, comp)},
                {"setY", makeFun<SetY>(caller, "setY", ecsRef, comp)},
                {"setWidth", makeFun<SetW>(caller, "setWidth", ecsRef, comp)},
                {"setHeight", makeFun<SetH>(caller, "setHeight", ecsRef, comp)}};
    }
}