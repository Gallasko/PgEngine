#pragma once

#include "Systems/basicsystems.h"
#include "2D/position.h"
#include "UI/textinput.h"
#include <vector>
#include "json.hpp"
#include <limits>

using namespace pg;

struct GeneratorComponent {
    std::string name;
    double baseCost;
    double costGrowth;
    double baseYield;
    int level = 0;
    
    struct Threshold {
        int every = 0;
        double prodMult = 1.0;
    } threshold;
    
    GeneratorComponent(const std::string& n = "Generator") 
        : name(n), baseCost(10), costGrowth(1.15), baseYield(0.5) {}
};

struct SimulationState {
    double currency = 0.0;
    double productionRate = 0.0;
    double elapsedTime = 0.0;
    bool isRunning = false;
    std::vector<std::pair<double, std::string>> eventLog;
    
    void reset() {
        currency = 0.0;
        productionRate = 0.0;
        elapsedTime = 0.0;
        isRunning = false;
        eventLog.clear();
    }
};

struct CalibrationParams {
    bool calibrateBaseCost = true;
    bool calibrateCostGrowth = true;
    bool calibrateBaseYield = true;
    int maxIterations = 100;
    double targetTime = 3600.0;
    double bestScore = std::numeric_limits<double>::max();
};

class IdleCalibratorSystem : public System<InitSys, Listener<TickEvent>> {
private:
    EntityRef mainPanel;
    EntityRef simulateButton;
    EntityRef calibrateButton;
    EntityRef addGeneratorButton;
    EntityRef saveButton;
    EntityRef loadButton;
    
    struct GeneratorRow {
        EntityRef nameInput;
        EntityRef baseCostInput;
        EntityRef costGrowthInput;
        EntityRef baseYieldInput;
        EntityRef deleteButton;
        EntityRef levelDisplay;
    };
    std::vector<GeneratorRow> generatorRows;
    
    EntityRef currencyDisplay;
    EntityRef rateDisplay;
    EntityRef timeDisplay;
    EntityRef logPanel;
    
    SimulationState simState;
    CalibrationParams calibParams;
    std::vector<GeneratorComponent> generators;
    
    float deltaTime = 0.0f;
    bool needsUIUpdate = false;
    
public:
    IdleCalibratorSystem();
    
    void init() override;
    void execute() override;
    void onEvent(const TickEvent& event) override;
    
private:
    void createMainPanel();
    void createGeneratorEditor();
    void createSimulationDisplay();
    void createCalibrationControls();
    
    void addGeneratorRow(const GeneratorComponent& gen = GeneratorComponent());
    void removeGeneratorRow(size_t index);
    void updateGeneratorFromUI(size_t index);
    void refreshGeneratorDisplay();
    
    void runSimulation();
    void stepSimulation(double dt);
    double calculateCost(const GeneratorComponent& gen) const;
    double calculateProduction(const GeneratorComponent& gen) const;
    double calculateROI(const GeneratorComponent& gen) const;
    bool canAfford(const GeneratorComponent& gen) const;
    void purchaseGenerator(size_t index);
    double calculateTimeToNextPurchase() const;
    
    void runCalibration();
    std::vector<double> packParameters() const;
    void unpackParameters(const std::vector<double>& params);
    double evaluateFitness(const std::vector<double>& params);
    
    void updateSimulationDisplay();
    void addLogEntry(const std::string& message);
    
    void saveConfig(const std::string& path);
    void loadConfig(const std::string& path);
    nlohmann::json generatorToJson(const GeneratorComponent& gen) const;
    GeneratorComponent generatorFromJson(const nlohmann::json& json) const;
};