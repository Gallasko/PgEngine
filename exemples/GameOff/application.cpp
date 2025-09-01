#include "application.h"

#include "Systems/basicsystems.h"
#include "Systems/tween.h"
#include "UI/ttftext.h"
#include "window.h"

#include "nexusscene.h"
#include "managenerator.h"
#include "gamefacts.h"
#include "achievement.h"
#include "gamelog.h"
#include "theme.h"

using namespace pg;

namespace
{
    static const char* const DOM = "IdleGame App";
}

GameApp::GameApp(const std::string& appName) : engine(appName)
{
    engine.setSetupFunction([](EntitySystem& ecs, Window& window)
    {
        // Core systems
        ecs.createSystem<FpsSystem>();
        ecs.createSystem<ThemeSystem>();
        ecs.createSystem<TweenSystem>();

        // TTF Text system for UI
        auto ttfSys = ecs.createSystem<TTFTextSystem>(window.masterRenderer);
        ttfSys->registerFont("res/font/Inter/static/Inter_28pt-Light.ttf");
        ttfSys->registerFont("res/font/Inter/static/Inter_28pt-Bold.ttf");
        ttfSys->registerFont("res/font/Inter/static/Inter_28pt-Italic.ttf");

        window.masterRenderer->processTextureRegister();

        // Idle game systems
        auto worldFacts = ecs.createSystem<WorldFacts>();
        auto achievementSys = ecs.createSystem<AchievementSys>();
        ecs.createSystem<GameLog>();
        ecs.createSystem<RessourceGeneratorSystem>();
        ecs.createSystem<ConverterSystem>();
        ecs.createSystem<NexusSystem>();

        // Set up default world facts
        worldFacts->setDefaultFact("mana", 0.0f);
        worldFacts->setDefaultFact("scrap", 0.0f);
        worldFacts->setDefaultFact("altar_touched", false);

        worldFacts->setDefaultFact("startTuto", true);
        worldFacts->setDefaultFact("altar_touched", false);
        worldFacts->setDefaultFact("mage_tier", 0);

        Achievement tutoStarted;

        tutoStarted.name = "TutoStarted";
        tutoStarted.prerequisiteFacts = { FactChecker{"startTuto", true, FactCheckEquality::Equal} };
        tutoStarted.rewardFacts = { StandardEvent{"gamelog", "message", "You just woke up in a strange place... \nYou only see a runic altar nearby."} };

        achievementSys->setDefaultAchievement(tutoStarted);

        Achievement knowledgeFirstCap;

        knowledgeFirstCap.name = "knowledgeFirstCap";
        knowledgeFirstCap.prerequisiteFacts = { FactChecker{"knowledge", 10, FactCheckEquality::GreaterEqual} };
        knowledgeFirstCap.rewardFacts = { StandardEvent{"gamelog", "message", "You seem to have hit a wall in your study.\nTaking some notes may help you in the future..."}, AddFact{"first_knowledge_cap", ElementType{true}} };

        achievementSys->setDefaultAchievement(knowledgeFirstCap);

        ecs.createSystem<GameLog>();

        ecs.createSystem<RessourceGeneratorSystem>();
        ecs.createSystem<ConverterSystem>();
        ecs.createSystem<NexusSystem>();

        ecs.succeed<AchievementSys, WorldFacts>();

        window.interpreter->interpretFromFile("main.pg");

        // Load the main idle scene
        ecs.getSystem<SceneElementSystem>()->loadSystemScene<NexusScene>();
    });
}

GameApp::~GameApp()
{
}

int GameApp::exec()
{
    return engine.exec();
}