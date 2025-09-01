#include "idlecalibratorsystem.h"
#include <sstream>

IdleCalibratorSystem::IdleCalibratorSystem() {
    generators.push_back(GeneratorComponent("Cookie Clicker"));
}

void IdleCalibratorSystem::init() {
    LOG_INFO("IdleCalibrator", "Minimal system initialized - it works!");
    
    // Start a simple simulation immediately
    runSimulation();
}

void IdleCalibratorSystem::execute() {
    // Nothing to do in execute for this minimal version
}

void IdleCalibratorSystem::onEvent(const TickEvent& event) {
    deltaTime += event.tick / 1000.0f;
    
    if (simState.isRunning && deltaTime >= 1.0f) {
        stepSimulation(deltaTime);
        deltaTime = 0.0f;
    }
}

// Minimal implementations of required methods
void IdleCalibratorSystem::createMainPanel() {}
void IdleCalibratorSystem::createGeneratorEditor() {}
void IdleCalibratorSystem::createSimulationDisplay() {}
void IdleCalibratorSystem::createCalibrationControls() {}

void IdleCalibratorSystem::addGeneratorRow(const GeneratorComponent& gen) {
    if (generators.size() <= generatorRows.size()) {
        generators.push_back(gen);
    }
}

void IdleCalibratorSystem::removeGeneratorRow(size_t index) {}
void IdleCalibratorSystem::updateGeneratorFromUI(size_t index) {}
void IdleCalibratorSystem::refreshGeneratorDisplay() {}
void IdleCalibratorSystem::updateSimulationDisplay() {}
double IdleCalibratorSystem::calculateTimeToNextPurchase() const { return 0.0; }

void IdleCalibratorSystem::runSimulation() {
    simState.reset();
    simState.isRunning = true;
    LOG_INFO("IdleCalibrator", "Simulation started!");
    
    // Run for 10 seconds
    for (int i = 0; i < 10; ++i) {
        stepSimulation(1.0);
    }
    
    simState.isRunning = false;
    LOG_INFO("IdleCalibrator", "Simulation completed! Final currency: " + std::to_string(simState.currency));
}

void IdleCalibratorSystem::stepSimulation(double dt) {
    simState.elapsedTime += dt;
    
    // Simple logic: earn money based on generator levels
    double totalProduction = 0.0;
    for (const auto& gen : generators) {
        totalProduction += calculateProduction(gen);
    }
    
    simState.productionRate = totalProduction;
    simState.currency += totalProduction * dt;
    
    // Try to buy more generators
    for (size_t i = 0; i < generators.size(); ++i) {
        if (canAfford(generators[i])) {
            purchaseGenerator(i);
            break;
        }
    }
}

double IdleCalibratorSystem::calculateCost(const GeneratorComponent& gen) const {
    return gen.baseCost * std::pow(gen.costGrowth, gen.level);
}

double IdleCalibratorSystem::calculateProduction(const GeneratorComponent& gen) const {
    return gen.baseYield * gen.level;
}

double IdleCalibratorSystem::calculateROI(const GeneratorComponent& gen) const {
    double cost = calculateCost(gen);
    if (cost <= 0) return 0.0;
    return gen.baseYield / cost;
}

bool IdleCalibratorSystem::canAfford(const GeneratorComponent& gen) const {
    return simState.currency >= calculateCost(gen);
}

void IdleCalibratorSystem::purchaseGenerator(size_t index) {
    if (index >= generators.size() || !canAfford(generators[index])) return;
    
    double cost = calculateCost(generators[index]);
    simState.currency -= cost;
    generators[index].level++;
    
    LOG_INFO("IdleCalibrator", "Bought " + generators[index].name + " level " + std::to_string(generators[index].level));
}

void IdleCalibratorSystem::addLogEntry(const std::string& message) {
    LOG_INFO("IdleCalibrator", message);
}

// Empty implementations for JSON and calibration
void IdleCalibratorSystem::runCalibration() {}
std::vector<double> IdleCalibratorSystem::packParameters() const { return std::vector<double>(); }
void IdleCalibratorSystem::unpackParameters(const std::vector<double>& params) {}
double IdleCalibratorSystem::evaluateFitness(const std::vector<double>& params) { return 0.0; }
void IdleCalibratorSystem::saveConfig(const std::string& path) {}
void IdleCalibratorSystem::loadConfig(const std::string& path) {}
nlohmann::json IdleCalibratorSystem::generatorToJson(const GeneratorComponent& gen) const { return nlohmann::json(); }
GeneratorComponent IdleCalibratorSystem::generatorFromJson(const nlohmann::json& json) const { return GeneratorComponent(); }