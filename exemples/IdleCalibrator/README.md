# Idle Game Calibrator

A complete C++ idle game calibrator system built for ColumbaEngine that helps developers tune the economics and progression of idle/incremental games.

## Features

### Core Functionality
- **Generator Management**: Create, edit, and remove generators with customizable parameters
- **Real-time Simulation**: Run simulations with greedy ROI-based purchasing strategy
- **Parameter Calibration**: Automatically tune game parameters to meet target goals
- **JSON Configuration**: Save and load complete game configurations
- **Visual Feedback**: Live display of currency, production rates, and event logs

### Generator Parameters
- **Base Cost**: Initial purchase price
- **Cost Growth**: Exponential growth factor for each level
- **Base Yield**: Production per second per level
- **Threshold Mechanics**: Bonus multipliers at specific level milestones

### Calibration System
- Genetic algorithm-based parameter optimization
- Configurable target times and currency goals
- Support for multi-parameter tuning (cost, growth, yield)
- Fitness evaluation based on economic progression

## Architecture

### ColumbaEngine Integration
- Built as a proper ColumbaEngine System using the `System<>` template
- Uses native UI components (`makeUiTexture`, `makeTTFTextInput`, `UiAnchor`)
- Follows ECS architecture patterns from the bouncing box example
- Integrates with engine's event system (`TickEvent`, `MouseLeftClick`)

### System Structure
```
IdleCalibratorSystem
├── UI Management
│   ├── Generator Editor (dynamic rows)
│   ├── Simulation Display (currency, rate, time)
│   ├── Control Buttons (simulate, calibrate, save/load)
│   └── Event Log Panel
├── Simulation Engine
│   ├── Greedy ROI Strategy
│   ├── Generator Economics
│   └── Time-stepped Progression
└── Calibration Engine
    ├── Parameter Packing/Unpacking
    ├── Fitness Evaluation
    └── Optimization Algorithm
```

## Building

### Prerequisites
- ColumbaEngine (pgengine) installed and available via pkg-config
- nlohmann/json library
- CMake 3.18+
- C++17 compiler

### Ubuntu/Debian Setup
```bash
sudo apt install nlohmann-json3-dev cmake build-essential
```

### Fedora Setup
```bash
sudo dnf install json-devel cmake gcc-c++
```

### Build Instructions
```bash
cd examples/IdleCalibrator
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Run
```bash
./IdleCalibrator
```

## Usage

### Basic Operation
1. **Add Generators**: Click "Add Generator" to create new income sources
2. **Edit Parameters**: Modify base cost, cost growth, and yield values
3. **Run Simulation**: Click "Simulate" to see economic progression
4. **Calibrate**: Click "Calibrate" to automatically optimize parameters
5. **Save/Load**: Export and import configurations via JSON files

### Configuration Files
The system includes a demo configuration at `res/configs/demo_idle.json` with:
- 4 balanced generators (Cookie Clicker, Grandma, Farm, Factory)
- Threshold mechanics for production bonuses
- Event system for timed multipliers
- Goal system for progression targets

### Example Configuration
```json
{
  "generators": [
    {
      "name": "Cookie Clicker",
      "baseCost": 10,
      "costGrowth": 1.15,
      "baseYield": 0.5,
      "threshold": {
        "every": 10,
        "prodMult": 2.0
      }
    }
  ],
  "calibration": {
    "maxIterations": 100,
    "targetTime": 3600,
    "targetCurrency": 1000000
  }
}
```

## Development

### Key Components

#### GeneratorComponent
```cpp
struct GeneratorComponent {
    std::string name;
    double baseCost;      // Initial cost
    double costGrowth;    // Growth factor (e.g., 1.15 = 15% increase)
    double baseYield;     // Production per level
    int level = 0;        // Current level
    
    struct Threshold {
        int every = 0;        // Trigger every N levels
        double prodMult = 1;  // Production multiplier
    } threshold;
};
```

#### Simulation Algorithm
The simulation uses a greedy ROI (Return on Investment) strategy:
1. Calculate ROI for each affordable generator
2. Purchase the generator with the highest ROI
3. Advance time to accumulate currency
4. Repeat until target time is reached

#### Calibration Process
1. **Parameter Packing**: Convert generator parameters to optimization vector
2. **Fitness Evaluation**: Run simulation with test parameters
3. **Score Calculation**: Compare result to target currency goal
4. **Optimization**: Use random variations to find better parameters
5. **Parameter Unpacking**: Apply best-found parameters to generators

### Extending the System

#### Adding New Generator Types
```cpp
// Extend GeneratorComponent with new mechanics
struct AdvancedGenerator : public GeneratorComponent {
    double prestigeBonus = 1.0;
    int prestigeLevel = 0;
    std::vector<Upgrade> availableUpgrades;
};
```

#### Custom Calibration Algorithms
```cpp
class GeneticCalibratorSystem : public IdleCalibratorSystem {
    void runGeneticCalibration();
    std::vector<Individual> createPopulation();
    void mutateIndividual(Individual& individual);
};
```

## Testing

The system includes comprehensive testing scenarios:

### Basic Functionality Tests
- Generator creation/deletion
- Parameter editing and validation
- Save/load configuration integrity
- UI responsiveness

### Simulation Tests
- Economic progression accuracy
- ROI calculation correctness
- Time advancement consistency
- Purchase decision logic

### Calibration Tests
- Parameter optimization convergence
- Fitness function accuracy
- Multi-parameter tuning stability

## Performance Considerations

- **Simulation Speed**: Uses time-stepped updates for real-time feedback
- **Memory Efficiency**: Component-based architecture minimizes allocations
- **UI Updates**: Batched updates prevent frame rate drops during simulation
- **Large-Scale Calibration**: Supports hundreds of iterations without blocking

## Troubleshooting

### Build Issues
- **Missing pgengine**: Ensure ColumbaEngine is installed and pkg-config can find it
- **JSON not found**: Install nlohmann-json3-dev package
- **CMake errors**: Verify CMake 3.18+ is installed

### Runtime Issues
- **Font not loading**: Check that Inter_28pt-Regular.ttf exists in res/font/
- **Config not loading**: Verify JSON file syntax and file permissions
- **UI not responding**: Check that mouse system is properly initialized

## License

This project follows the same license as ColumbaEngine. See the main engine documentation for details.

## Contributing

1. Follow ColumbaEngine coding conventions
2. Use existing component patterns
3. Maintain ECS architecture principles
4. Add tests for new features
5. Update documentation for API changes