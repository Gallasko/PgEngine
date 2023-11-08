#include "focusable.h"

#include "serialization.h"

namespace pg
{
    template<>
    void serialize(Archive& archive, const FocusableComponent& component)
    {
        archive.startSerialization("Mouse Left Click Component");

        serialize(archive, "entityId", component.entityId);

        serialize(archive, "alwaysFocused", component.alwaysFocus);
        serialize(archive, "focused", component.focused);

        archive.endSerialization();
    }
}