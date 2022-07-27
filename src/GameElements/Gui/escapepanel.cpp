#include "escapepanel.h"
#include "UI/texture.h"
#include "Engine/logger.h"
#include "Renderer/renderer.h"

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
    LOG_THIS_MEMBER(DOM);

    if(background != nullptr)
    {
        background->setTopAnchor(this->top);
        background->setLeftAnchor(this->left);
    }
}

void EscapePanel::show()
{
    LOG_THIS_MEMBER(DOM);

    UiComponent::show();

    if(background != nullptr)
        background->show();
}

void EscapePanel::hide()
{
    LOG_THIS_MEMBER(DOM);

    UiComponent::hide();

    if(background != nullptr)
        background->hide();
}