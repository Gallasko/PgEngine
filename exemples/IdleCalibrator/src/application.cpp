#include "application.h"
#include "Systems/basicsystems.h"
#include "systems/idlecalibratorsystem.h"

using namespace pg;

GameApp::GameApp(const std::string& appName) : engine(appName) {
    engine.setSetupFunction([this](EntitySystem& ecs, Window& window) {
        ecs.createSystem<FpsSystem>();
        ecs.createSystem<IdleCalibratorSystem>();
        
        LOG_INFO("GameApp", "Systems initialized");
    });
}

int GameApp::exec() {
    return engine.exec();
}