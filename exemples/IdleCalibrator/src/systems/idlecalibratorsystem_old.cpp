#include "idlecalibratorsystem.h"
#include "2D/texture.h"
#include "UI/ttftext.h"
#include "UI/focusable.h"
#include <sstream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <random>
#include <cmath>

IdleCalibratorSystem::IdleCalibratorSystem() {
    generators.push_back(GeneratorComponent("Cookie Clicker"));
}

void IdleCalibratorSystem::init() {
    createMainPanel();
    createGeneratorEditor();
    createSimulationDisplay();
    createCalibrationControls();
    
    addGeneratorRow(generators[0]);
    
    loadConfig("res/configs/demo_idle.json");
    
    LOG_INFO("IdleCalibrator", "System initialized");
}

void IdleCalibratorSystem::execute() {
    if (needsUIUpdate) {
        updateSimulationDisplay();
        refreshGeneratorDisplay();
        needsUIUpdate = false;
    }
}

void IdleCalibratorSystem::createMainPanel() {
    mainPanel = makeUiTexture(ecsRef, 1200, 800, Color(0.2f, 0.2f, 0.3f, 1.0f)).entity;
    auto panelPos = mainPanel.get<PositionComponent>();
    panelPos->setX(50);
    panelPos->setY(50);
    
    auto title = makeTTFText(ecsRef, 0, 0, "Idle Game Calibrator", 
                            "res/font/Inter/static/Inter_28pt-Regular.ttf", 1.2f);
    auto titleAnchor = title.get<UiAnchor>();
    titleAnchor->setTopAnchor(mainPanel.get<UiAnchor>()->top);
    titleAnchor->setTopMargin(20);
    titleAnchor->centeredIn(mainPanel.get<UiAnchor>());
}

void IdleCalibratorSystem::createGeneratorEditor() {
    auto genHeader = makeTTFText(ecsRef, 0, 0, "Generators", 
                                "res/font/Inter/static/Inter_28pt-Regular.ttf", 1.0f);
    auto genHeaderAnchor = genHeader.get<UiAnchor>();
    genHeaderAnchor->setTopAnchor(mainPanel.get<UiAnchor>()->top);
    genHeaderAnchor->setTopMargin(80);
    genHeaderAnchor->setLeftAnchor(mainPanel.get<UiAnchor>()->left);
    genHeaderAnchor->setLeftMargin(40);
    
    addGeneratorButton = makeUiTexture(ecsRef, 140, 35, Color(0.3f, 0.6f, 0.3f, 1.0f)).entity;
    auto addBtnAnchor = addGeneratorButton.get<UiAnchor>();
    addBtnAnchor->setTopAnchor(genHeaderAnchor->bottom);
    addBtnAnchor->setTopMargin(10);
    addBtnAnchor->setLeftAnchor(genHeaderAnchor->left);
    
    auto btnText = makeTTFText(ecsRef, 0, 0, "Add Generator", 
                              "res/font/Inter/static/Inter_28pt-Regular.ttf", 0.7f);
    auto btnTextAnchor = btnText.get<UiAnchor>();
    btnTextAnchor->centeredIn(addGeneratorButton.get<UiAnchor>());
    
    ecsRef->attach<MouseLeftClickComponent>(addGeneratorButton, [this](Input*, double) {
        addGeneratorRow();
    });
}

void IdleCalibratorSystem::createSimulationDisplay() {
    auto simHeader = makeTTFText(ecsRef, 0, 0, "Simulation", 
                                "res/font/Inter/static/Inter_28pt-Regular.ttf", 1.0f);
    auto simHeaderAnchor = simHeader.get<UiAnchor>();
    simHeaderAnchor->setTopAnchor(mainPanel.get<UiAnchor>()->top);
    simHeaderAnchor->setTopMargin(80);
    simHeaderAnchor->setRightAnchor(mainPanel.get<UiAnchor>()->right);
    simHeaderAnchor->setRightMargin(300);
    
    currencyDisplay = makeTTFText(ecsRef, 0, 0, "Currency: $0.00", 
                                 "res/font/Inter/static/Inter_28pt-Regular.ttf", 0.8f).entity;
    auto currencyAnchor = currencyDisplay.get<UiAnchor>();
    currencyAnchor->setTopAnchor(simHeaderAnchor->bottom);
    currencyAnchor->setTopMargin(20);
    currencyAnchor->setLeftAnchor(simHeaderAnchor->left);
    
    rateDisplay = makeTTFText(ecsRef, 0, 0, "Rate: $0.00/sec", 
                             "res/font/Inter/static/Inter_28pt-Regular.ttf", 0.8f).entity;
    auto rateAnchor = rateDisplay.get<UiAnchor>();
    rateAnchor->setTopAnchor(currencyAnchor->bottom);
    rateAnchor->setTopMargin(10);
    rateAnchor->setLeftAnchor(currencyAnchor->left);
    
    timeDisplay = makeTTFText(ecsRef, 0, 0, "Time: 0.0s", 
                             "res/font/Inter/static/Inter_28pt-Regular.ttf", 0.8f).entity;
    auto timeAnchor = timeDisplay.get<UiAnchor>();
    timeAnchor->setTopAnchor(rateAnchor->bottom);
    timeAnchor->setTopMargin(10);
    timeAnchor->setLeftAnchor(rateAnchor->left);
    
    logPanel = makeUiTexture(ecsRef, 280, 300, Color(0.1f, 0.1f, 0.1f, 1.0f)).entity;
    auto logAnchor = logPanel.get<UiAnchor>();
    logAnchor->setTopAnchor(timeAnchor->bottom);
    logAnchor->setTopMargin(20);
    logAnchor->setLeftAnchor(timeAnchor->left);
}

void IdleCalibratorSystem::createCalibrationControls() {
    simulateButton = makeUiTexture(ecsRef, 100, 35, Color(0.3f, 0.3f, 0.6f, 1.0f)).entity;
    auto simBtnAnchor = simulateButton.get<UiAnchor>();
    simBtnAnchor->setBottomAnchor(mainPanel.get<UiAnchor>()->bottom);
    simBtnAnchor->setBottomMargin(100);
    simBtnAnchor->setLeftAnchor(mainPanel.get<UiAnchor>()->left);
    simBtnAnchor->setLeftMargin(40);
    
    auto simBtnText = makeTTFText(ecsRef, 0, 0, "Simulate", 
                                 "res/font/Inter/static/Inter_28pt-Regular.ttf", 0.7f);
    auto simBtnTextAnchor = simBtnText.get<UiAnchor>();
    simBtnTextAnchor->centeredIn(simulateButton.get<UiAnchor>());
    
    calibrateButton = makeUiTexture(ecsRef, 100, 35, Color(0.6f, 0.3f, 0.3f, 1.0f)).entity;
    auto calBtnAnchor = calibrateButton.get<UiAnchor>();
    calBtnAnchor->setTopAnchor(simBtnAnchor->top);
    calBtnAnchor->setLeftAnchor(simBtnAnchor->right);
    calBtnAnchor->setLeftMargin(20);
    
    auto calBtnText = makeTTFText(ecsRef, 0, 0, "Calibrate", 
                                 "res/font/Inter/static/Inter_28pt-Regular.ttf", 0.7f);
    auto calBtnTextAnchor = calBtnText.get<UiAnchor>();
    calBtnTextAnchor->centeredIn(calibrateButton.get<UiAnchor>());
    
    saveButton = makeUiTexture(ecsRef, 80, 35, Color(0.4f, 0.4f, 0.4f, 1.0f)).entity;
    auto saveBtnAnchor = saveButton.get<UiAnchor>();
    saveBtnAnchor->setTopAnchor(calBtnAnchor->top);
    saveBtnAnchor->setLeftAnchor(calBtnAnchor->right);
    saveBtnAnchor->setLeftMargin(20);
    
    auto saveBtnText = makeTTFText(ecsRef, 0, 0, "Save", 
                                  "res/font/Inter/static/Inter_28pt-Regular.ttf", 0.7f);
    auto saveBtnTextAnchor = saveBtnText.get<UiAnchor>();
    saveBtnTextAnchor->centeredIn(saveButton.get<UiAnchor>());
    
    loadButton = makeUiTexture(ecsRef, 80, 35, Color(0.4f, 0.4f, 0.4f, 1.0f)).entity;
    auto loadBtnAnchor = loadButton.get<UiAnchor>();
    loadBtnAnchor->setTopAnchor(saveBtnAnchor->top);
    loadBtnAnchor->setLeftAnchor(saveBtnAnchor->right);
    loadBtnAnchor->setLeftMargin(20);
    
    auto loadBtnText = makeTTFText(ecsRef, 0, 0, "Load", 
                                  "res/font/Inter/static/Inter_28pt-Regular.ttf", 0.7f);
    auto loadBtnTextAnchor = loadBtnText.get<UiAnchor>();
    loadBtnTextAnchor->centeredIn(loadButton.get<UiAnchor>());
    
    ecsRef->attach<MouseLeftClickComponent>(simulateButton, [this](Input*, double) {
        runSimulation();
    });
    
    ecsRef->attach<MouseLeftClickComponent>(calibrateButton, [this](Input*, double) {
        runCalibration();
    });
    
    ecsRef->attach<MouseLeftClickComponent>(saveButton, [this](Input*, double) {
        saveConfig("res/configs/current_config.json");
        addLogEntry("Configuration saved");
    });
    
    ecsRef->attach<MouseLeftClickComponent>(loadButton, [this](Input*, double) {
        loadConfig("res/configs/demo_idle.json");
        addLogEntry("Configuration loaded");
    });
}

void IdleCalibratorSystem::addGeneratorRow(const GeneratorComponent& gen) {
    GeneratorRow row;
    
    float yOffset = 150 + generatorRows.size() * 45;
    
    row.nameInput = makeTTFTextInput(ecsRef, 0, 0,
        StandardEvent("GeneratorNameChange"),
        "res/font/Inter/static/Inter_28pt-Regular.ttf",
        gen.name, 0.7f).entity;
    auto nameAnchor = row.nameInput.get<UiAnchor>();
    nameAnchor->setTopAnchor(mainPanel.get<UiAnchor>()->top);
    nameAnchor->setTopMargin(yOffset);
    nameAnchor->setLeftAnchor(mainPanel.get<UiAnchor>()->left);
    nameAnchor->setLeftMargin(40);
    
    row.baseCostInput = makeTTFTextInput(ecsRef, 0, 0,
        StandardEvent("BaseCostChange"),
        "res/font/Inter/static/Inter_28pt-Regular.ttf",
        std::to_string(gen.baseCost), 0.7f).entity;
    auto costAnchor = row.baseCostInput.get<UiAnchor>();
    costAnchor->setTopAnchor(nameAnchor->top);
    costAnchor->setLeftAnchor(nameAnchor->right);
    costAnchor->setLeftMargin(20);
    
    row.costGrowthInput = makeTTFTextInput(ecsRef, 0, 0,
        StandardEvent("CostGrowthChange"),
        "res/font/Inter/static/Inter_28pt-Regular.ttf",
        std::to_string(gen.costGrowth), 0.7f).entity;
    auto growthAnchor = row.costGrowthInput.get<UiAnchor>();
    growthAnchor->setTopAnchor(costAnchor->top);
    growthAnchor->setLeftAnchor(costAnchor->right);
    growthAnchor->setLeftMargin(20);
    
    row.baseYieldInput = makeTTFTextInput(ecsRef, 0, 0,
        StandardEvent("BaseYieldChange"),
        "res/font/Inter/static/Inter_28pt-Regular.ttf",
        std::to_string(gen.baseYield), 0.7f).entity;
    auto yieldAnchor = row.baseYieldInput.get<UiAnchor>();
    yieldAnchor->setTopAnchor(growthAnchor->top);
    yieldAnchor->setLeftAnchor(growthAnchor->right);
    yieldAnchor->setLeftMargin(20);
    
    row.levelDisplay = makeTTFText(ecsRef, 0, 0, 
        "Lvl: " + std::to_string(gen.level),
        "res/font/Inter/static/Inter_28pt-Regular.ttf", 0.7f).entity;
    auto levelAnchor = row.levelDisplay.get<UiAnchor>();
    levelAnchor->setTopAnchor(yieldAnchor->top);
    levelAnchor->setLeftAnchor(yieldAnchor->right);
    levelAnchor->setLeftMargin(20);
    
    row.deleteButton = makeUiTexture(ecsRef, 60, 25, Color(0.6f, 0.2f, 0.2f, 1.0f)).entity;
    auto delAnchor = row.deleteButton.get<UiAnchor>();
    delAnchor->setTopAnchor(levelAnchor->top);
    delAnchor->setLeftAnchor(levelAnchor->right);
    delAnchor->setLeftMargin(20);
    
    auto delText = makeTTFText(ecsRef, 0, 0, "Del", 
                              "res/font/Inter/static/Inter_28pt-Regular.ttf", 0.6f);
    auto delTextAnchor = delText.get<UiAnchor>();
    delTextAnchor->centeredIn(row.deleteButton.get<UiAnchor>());
    
    size_t index = generatorRows.size();
    ecsRef->attach<MouseLeftClickComponent>(row.deleteButton, [this, index](Input*, double) {
        if (index < generatorRows.size()) {
            removeGeneratorRow(index);
        }
    });
    
    if (generators.size() <= generatorRows.size()) {
        generators.push_back(gen);
    }
    
    generatorRows.push_back(row);
}

void IdleCalibratorSystem::removeGeneratorRow(size_t index) {
    if (index >= generatorRows.size() || generatorRows.size() <= 1) return;
    
    auto& row = generatorRows[index];
    ecsRef->destroyEntity(row.nameInput);
    ecsRef->destroyEntity(row.baseCostInput);
    ecsRef->destroyEntity(row.costGrowthInput);
    ecsRef->destroyEntity(row.baseYieldInput);
    ecsRef->destroyEntity(row.levelDisplay);
    ecsRef->destroyEntity(row.deleteButton);
    
    generatorRows.erase(generatorRows.begin() + index);
    generators.erase(generators.begin() + index);
}

void IdleCalibratorSystem::runSimulation() {
    simState.reset();
    
    for (auto& gen : generators) {
        gen.level = 0;
    }
    
    simState.isRunning = true;
    addLogEntry("Starting simulation...");
    
    const double timeStep = 0.1;
    const int maxSteps = static_cast<int>(calibParams.targetTime / timeStep);
    
    for (int step = 0; step < maxSteps && simState.isRunning; ++step) {
        stepSimulation(timeStep);
        
        if (step % 100 == 0) {
            needsUIUpdate = true;
        }
    }
    
    simState.isRunning = false;
    needsUIUpdate = true;
    addLogEntry("Simulation completed");
}

void IdleCalibratorSystem::stepSimulation(double dt) {
    simState.elapsedTime += dt;
    
    double totalProduction = 0.0;
    for (const auto& gen : generators) {
        totalProduction += calculateProduction(gen);
    }
    
    simState.productionRate = totalProduction;
    simState.currency += totalProduction * dt;
    
    int bestGen = -1;
    double bestROI = 0.0;
    
    for (size_t i = 0; i < generators.size(); ++i) {
        if (canAfford(generators[i])) {
            double roi = calculateROI(generators[i]);
            if (roi > bestROI) {
                bestROI = roi;
                bestGen = static_cast<int>(i);
            }
        }
    }
    
    if (bestGen >= 0) {
        purchaseGenerator(bestGen);
    }
}

double IdleCalibratorSystem::calculateCost(const GeneratorComponent& gen) const {
    return gen.baseCost * std::pow(gen.costGrowth, gen.level);
}

double IdleCalibratorSystem::calculateProduction(const GeneratorComponent& gen) const {
    if (gen.level == 0) return 0.0;
    
    double baseProduction = gen.baseYield * gen.level;
    
    if (gen.threshold.every > 0) {
        int thresholdCount = gen.level / gen.threshold.every;
        baseProduction *= std::pow(gen.threshold.prodMult, thresholdCount);
    }
    
    return baseProduction;
}

double IdleCalibratorSystem::calculateROI(const GeneratorComponent& gen) const {
    double cost = calculateCost(gen);
    if (cost <= 0) return 0.0;
    
    double currentProduction = calculateProduction(gen);
    
    GeneratorComponent tempGen = gen;
    tempGen.level++;
    double newProduction = calculateProduction(tempGen);
    
    double productionIncrease = newProduction - currentProduction;
    
    return productionIncrease / cost;
}

bool IdleCalibratorSystem::canAfford(const GeneratorComponent& gen) const {
    return simState.currency >= calculateCost(gen);
}

void IdleCalibratorSystem::purchaseGenerator(size_t index) {
    if (index >= generators.size() || !canAfford(generators[index])) return;
    
    double cost = calculateCost(generators[index]);
    simState.currency -= cost;
    generators[index].level++;
    
    std::ostringstream oss;
    oss << "Bought " << generators[index].name << " (Lvl " << generators[index].level << ")";
    addLogEntry(oss.str());
}

void IdleCalibratorSystem::runCalibration() {
    addLogEntry("Starting calibration...");
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    auto originalParams = packParameters();
    double bestScore = std::numeric_limits<double>::max();
    std::vector<double> bestParams = originalParams;
    
    for (int iteration = 0; iteration < calibParams.maxIterations; ++iteration) {
        std::vector<double> testParams = originalParams;
        
        for (size_t i = 0; i < testParams.size(); ++i) {
            double variation = (dis(gen) - 0.5) * 0.2;
            testParams[i] *= (1.0 + variation);
            testParams[i] = std::max(0.01, testParams[i]);
        }
        
        double score = evaluateFitness(testParams);
        
        if (score < bestScore) {
            bestScore = score;
            bestParams = testParams;
        }
        
        if (iteration % 10 == 0) {
            std::ostringstream oss;
            oss << "Iteration " << iteration << ", Best: " << std::fixed << std::setprecision(2) << bestScore;
            addLogEntry(oss.str());
        }
    }
    
    unpackParameters(bestParams);
    needsUIUpdate = true;
    addLogEntry("Calibration completed!");
}

std::vector<double> IdleCalibratorSystem::packParameters() const {
    std::vector<double> params;
    
    for (const auto& gen : generators) {
        if (calibParams.calibrateBaseCost) params.push_back(gen.baseCost);
        if (calibParams.calibrateCostGrowth) params.push_back(gen.costGrowth);
        if (calibParams.calibrateBaseYield) params.push_back(gen.baseYield);
    }
    
    return params;
}

void IdleCalibratorSystem::unpackParameters(const std::vector<double>& params) {
    size_t paramIndex = 0;
    
    for (auto& gen : generators) {
        if (calibParams.calibrateBaseCost && paramIndex < params.size()) {
            gen.baseCost = params[paramIndex++];
        }
        if (calibParams.calibrateCostGrowth && paramIndex < params.size()) {
            gen.costGrowth = params[paramIndex++];
        }
        if (calibParams.calibrateBaseYield && paramIndex < params.size()) {
            gen.baseYield = params[paramIndex++];
        }
    }
}

double IdleCalibratorSystem::evaluateFitness(const std::vector<double>& params) {
    auto originalGenerators = generators;
    unpackParameters(params);
    
    SimulationState testState;
    std::vector<GeneratorComponent> testGens = generators;
    
    for (auto& gen : testGens) {
        gen.level = 0;
    }
    
    const double timeStep = 1.0;
    const int maxSteps = static_cast<int>(calibParams.targetTime / timeStep);
    
    for (int step = 0; step < maxSteps; ++step) {
        double totalProduction = 0.0;
        for (const auto& gen : testGens) {
            totalProduction += calculateProduction(gen);
        }
        
        testState.currency += totalProduction * timeStep;
        
        int bestGen = -1;
        double bestROI = 0.0;
        
        for (size_t i = 0; i < testGens.size(); ++i) {
            double cost = calculateCost(testGens[i]);
            if (testState.currency >= cost) {
                double roi = calculateROI(testGens[i]);
                if (roi > bestROI) {
                    bestROI = roi;
                    bestGen = static_cast<int>(i);
                }
            }
        }
        
        if (bestGen >= 0) {
            double cost = calculateCost(testGens[bestGen]);
            testState.currency -= cost;
            testGens[bestGen].level++;
        }
    }
    
    generators = originalGenerators;
    
    double targetCurrency = 1000000.0;
    return std::abs(testState.currency - targetCurrency);
}

void IdleCalibratorSystem::updateSimulationDisplay() {
    if (currencyDisplay.isValid()) {
        std::ostringstream oss;
        oss << "Currency: $" << std::fixed << std::setprecision(2) << simState.currency;
        ecsRef->get<TextComponent>(currencyDisplay)->text = oss.str();
    }
    
    if (rateDisplay.isValid()) {
        std::ostringstream oss;
        oss << "Rate: $" << std::fixed << std::setprecision(2) << simState.productionRate << "/sec";
        ecsRef->get<TextComponent>(rateDisplay)->text = oss.str();
    }
    
    if (timeDisplay.isValid()) {
        std::ostringstream oss;
        oss << "Time: " << std::fixed << std::setprecision(1) << simState.elapsedTime << "s";
        ecsRef->get<TextComponent>(timeDisplay)->text = oss.str();
    }
}

void IdleCalibratorSystem::refreshGeneratorDisplay() {
    for (size_t i = 0; i < generatorRows.size() && i < generators.size(); ++i) {
        if (generatorRows[i].levelDisplay.isValid()) {
            std::string levelText = "Lvl: " + std::to_string(generators[i].level);
            ecsRef->get<TextComponent>(generatorRows[i].levelDisplay)->text = levelText;
        }
    }
}

void IdleCalibratorSystem::addLogEntry(const std::string& message) {
    simState.eventLog.push_back({simState.elapsedTime, message});
    
    if (simState.eventLog.size() > 10) {
        simState.eventLog.erase(simState.eventLog.begin());
    }
    
    LOG_INFO("IdleCalibrator", message);
}

void IdleCalibratorSystem::onEvent(const TickEvent& event) {
    deltaTime += event.tick / 1000.0f;
    
    if (simState.isRunning && deltaTime >= 0.1f) {
        stepSimulation(deltaTime);
        needsUIUpdate = true;
        deltaTime = 0.0f;
    }
}

void IdleCalibratorSystem::onEvent(const MouseLeftClick& event) {
}

void IdleCalibratorSystem::saveConfig(const std::string& path) {
    nlohmann::json config;
    config["generators"] = nlohmann::json::array();
    
    for (const auto& gen : generators) {
        config["generators"].push_back(generatorToJson(gen));
    }
    
    config["calibration"]["maxIterations"] = calibParams.maxIterations;
    config["calibration"]["targetTime"] = calibParams.targetTime;
    config["calibration"]["enabledParams"]["baseCost"] = calibParams.calibrateBaseCost;
    config["calibration"]["enabledParams"]["costGrowth"] = calibParams.calibrateCostGrowth;
    config["calibration"]["enabledParams"]["baseYield"] = calibParams.calibrateBaseYield;
    
    std::ofstream file(path);
    if (file.is_open()) {
        file << config.dump(4);
        file.close();
    }
}

void IdleCalibratorSystem::loadConfig(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return;
    
    nlohmann::json config;
    file >> config;
    file.close();
    
    if (config.contains("generators")) {
        generators.clear();
        for (const auto& genJson : config["generators"]) {
            generators.push_back(generatorFromJson(genJson));
        }
    }
    
    if (config.contains("calibration")) {
        auto& cal = config["calibration"];
        if (cal.contains("maxIterations")) calibParams.maxIterations = cal["maxIterations"];
        if (cal.contains("targetTime")) calibParams.targetTime = cal["targetTime"];
        
        if (cal.contains("enabledParams")) {
            auto& params = cal["enabledParams"];
            if (params.contains("baseCost")) calibParams.calibrateBaseCost = params["baseCost"];
            if (params.contains("costGrowth")) calibParams.calibrateCostGrowth = params["costGrowth"];
            if (params.contains("baseYield")) calibParams.calibrateBaseYield = params["baseYield"];
        }
    }
    
    needsUIUpdate = true;
}

nlohmann::json IdleCalibratorSystem::generatorToJson(const GeneratorComponent& gen) const {
    nlohmann::json j;
    j["name"] = gen.name;
    j["baseCost"] = gen.baseCost;
    j["costGrowth"] = gen.costGrowth;
    j["baseYield"] = gen.baseYield;
    j["threshold"]["every"] = gen.threshold.every;
    j["threshold"]["prodMult"] = gen.threshold.prodMult;
    return j;
}

GeneratorComponent IdleCalibratorSystem::generatorFromJson(const nlohmann::json& json) const {
    GeneratorComponent gen;
    
    if (json.contains("name")) gen.name = json["name"];
    if (json.contains("baseCost")) gen.baseCost = json["baseCost"];
    if (json.contains("costGrowth")) gen.costGrowth = json["costGrowth"];
    if (json.contains("baseYield")) gen.baseYield = json["baseYield"];
    
    if (json.contains("threshold")) {
        auto& threshold = json["threshold"];
        if (threshold.contains("every")) gen.threshold.every = threshold["every"];
        if (threshold.contains("prodMult")) gen.threshold.prodMult = threshold["prodMult"];
    }
    
    return gen;
}