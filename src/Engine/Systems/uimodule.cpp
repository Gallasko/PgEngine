#include "uimodule.h"

namespace pg
{
    std::vector<SysListElement> uiElementFunctionsList(const Function *caller, EntitySystem *ecsRef, const CompRef<UiComponent>& comp)
    {
        return {{"x", static_cast<UiSize>(comp->pos.x)},
                {"y", static_cast<UiSize>(comp->pos.y)},
                {"w", comp->width},
                {"h", comp->height},
                {"setX", makeFun<SetX>(caller, "setX", ecsRef, comp)},
                {"setY", makeFun<SetY>(caller, "setY", ecsRef, comp)},
                {"setWidth", makeFun<SetW>(caller, "setWidth", ecsRef, comp)},
                {"setHeight", makeFun<SetH>(caller, "setHeight", ecsRef, comp)}};
    }
}