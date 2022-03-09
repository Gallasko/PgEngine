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

EscapePanel::EscapePanel(TextureComponent* background) : background(background)
{
    if(background != nullptr)
    {
        background->setTopAnchor(this->top);
        background->setLeftAnchor(this->left);
    }
}

void EscapePanel::show()
{
    if(background != nullptr)
        background->show();
}


#include <iostream>

void EscapePanel::hide()
{
    if(background != nullptr)
        background->hide();
}