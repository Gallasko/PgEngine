#include "escapepanel.h"

using namespace pg;

namespace
{
    const char * DOM = "Escape Panel";
}

namespace pg
{
    template<>
    void renderer(MasterRenderer* masterRenderer, EscapePanel* escapePanel)
    {
        if(escapePanel->background != nullptr)
            masterRenderer->render(escapePanel->background);
    }
}

EscapePanel::EscapePanel(TextureComponent* background)
{

}