#pragma once

// C++ standard
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <algorithm>
#include <functional>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <variant>

#include "Maths/geometry.h"

#include "logger.h"
#include "configuration.h"
#include "serialization.h"
#include "pgconstant.h"

// #include <vector>
// #include <unordered_map>
// #include <algorithm>

// If issues happens while working on system.h, comment this line !
#include "ECS/system.h"
#include "ECS/entitysystem.h"
#include "Renderer/renderer.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

// #include <taskflow.hpp>
