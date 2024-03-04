cmake_minimum_required(VERSION 3.20)
#cmake_minimum_required(VERSION 3.13)

project(PgEngine VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

enable_testing()

set(TF_BUILD_EXAMPLES OFF)
set(TF_BUILD_TESTS OFF)
set(TF_BUILD_BENCHMARKS OFF)

set(SDL2MIXER_OPUS OFF)
set(SDL2MIXER_FLAC_LIBFLAC OFF)
set(SDL2MIXER_MOD_MODPLUG OFF)
set(SDL2MIXER_MIDI_FLUIDSYNTH OFF)

set(BUILD_SHARED_LIBS OFF)
set(SDL2_DISABLE_INSTALL ON)
set(SDL_TEST OFF)

add_compile_options(-DGLEW_STATIC)

set(SDL2_DIR "import/SDL2-2.28.5")
set(SDL2MIXER_DIR "import/SDL2_mixer-2.6.3")
set(SDL2TTF_DIR "import/SDL2_ttf-2.20.2")
set(GLM_DIR "import/glm")
set(TASKFLOW_DIR "import/taskflow-3.6.0")
set(GTEST "import/googletest-1.14.0")
set(GLEW_DIR "import/glew-cmake")

find_package(OpenGL REQUIRED)

add_subdirectory(${SDL2_DIR})
add_subdirectory(${SDL2MIXER_DIR})

if (NOT EMSCRIPTEN)
    add_subdirectory(${GLEW_DIR})
endif()

# Todo make this work
# add_subdirectory(${SDL2TTF_DIR})

add_subdirectory(${GLM_DIR})
add_subdirectory(${TASKFLOW_DIR})
add_subdirectory(${GTEST})

include_directories(${GLM_DIR}/glm)
include_directories(${SDL2_DIR}/include)
include_directories(${TASKFLOW_DIR}/taskflow)

if (NOT EMSCRIPTEN)
    include_directories(${GLEW_DIR}/include)
endif()


if (NOT EMSCRIPTEN)
    set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -g -pthread -Wa,-mbig-obj -ggdb -DDEBUG")
else()
    set(CMAKE_C_FLAGS_DEBUG "-Wall -Wextra -g -pthread -ggdb -DDEBUG -DGLEW_STATIC USE_PTHREADS -m32")
    set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -g -pthread -ggdb -DDEBUG -DGLEW_STATIC USE_PTHREADS -m32")
endif()

# set(CMAKE_CXX_FLAGS_DEBUG "-std=c++17 -Wall -Wextra -g -pthread -Wa,-mbig-obj -s -O3 -DNDEBUG -mwindows")

if (NOT EMSCRIPTEN)
# set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wextra -pthread -Wa,-mbig-obj -s -O3 -DNDEBUG -mwindows")
    set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wextra -pthread -Wa,-mbig-obj -s -O3 -DNDEBUG")
else()
    set(CMAKE_C_FLAGS_RELEASE "-Wall -Wextra -pthread -O3 -DNDEBUG -DGLEW_STATIC -s USE_PTHREADS -m32 -sGL_ENABLE_GET_PROC_ADDRESS")
    set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wextra -pthread -O3 -DNDEBUG -DGLEW_STATIC -s USE_PTHREADS -m32 -sGL_ENABLE_GET_PROC_ADDRESS")
endif()

add_executable(PgEngine src/main.cpp)

add_executable(TetrisClone src/Tetris/tetris.cpp)

target_sources(PgEngine PRIVATE
        src/application.cpp
        src/application.h

        src/Engine/configuration.h
        src/Engine/configuration.cpp
        src/Engine/constant.h
        src/Engine/gl_debug.hpp
        src/Engine/logger.h
        src/Engine/logger.cpp
        src/Engine/serialization.cpp
        src/Engine/serialization.h
        src/Engine/window.cpp
        src/Engine/window.h

        src/Engine/2D/simple2dobject.cpp
        src/Engine/2D/simple2dobject.h
        src/Engine/2D/texture.cpp
        src/Engine/2D/texture.h

        src/Engine/Audio/audiomodule.h
        src/Engine/Audio/audiosystem.cpp
        src/Engine/Audio/audiosystem.h

        src/Engine/ECS/callable.h
        src/Engine/ECS/commanddispatcher.cpp
        src/Engine/ECS/commanddispatcher.h
        src/Engine/ECS/componentregistry.cpp
        src/Engine/ECS/componentregistry.h
        src/Engine/ECS/ecsmodule.h
        src/Engine/ECS/entity.cpp
        src/Engine/ECS/entity.h
        src/Engine/ECS/entitycreationsystem.h
        src/Engine/ECS/entitysystem.cpp
        src/Engine/ECS/entitysystem.h
        src/Engine/ECS/eventlistener.h
        src/Engine/ECS/group.cpp
        src/Engine/ECS/group.h
        src/Engine/ECS/identifiers.h
        src/Engine/ECS/loggersystem.h
        src/Engine/ECS/sparseset.cpp
        src/Engine/ECS/sparseset.h
        src/Engine/ECS/system.cpp
        src/Engine/ECS/system.h
        src/Engine/ECS/uniqueid.cpp
        src/Engine/ECS/uniqueid.h

        src/Engine/Event/eventloop.cpp
        src/Engine/Event/eventloop.h

        src/Engine/Files/filemanager.cpp
        src/Engine/Files/filemanager.h
        src/Engine/Files/fileparser.cpp
        src/Engine/Files/fileparser.h

        src/Engine/Helpers/date.h
        src/Engine/Helpers/detector.h
        src/Engine/Helpers/openglobject.cpp
        src/Engine/Helpers/openglobject.h

        src/Engine/Input/input.cpp
        src/Engine/Input/input.h
        src/Engine/Input/inputcomponent.cpp
        src/Engine/Input/inputcomponent.h
        src/Engine/Input/inputmodule.h

        src/Engine/Interpreter/environment.cpp
        src/Engine/Interpreter/environment.h
        src/Engine/Interpreter/expression.cpp
        src/Engine/Interpreter/expression.h
        src/Engine/Interpreter/interpreter.cpp
        src/Engine/Interpreter/interpreter.h
        src/Engine/Interpreter/interpretersystem.cpp
        src/Engine/Interpreter/interpretersystem.h
        src/Engine/Interpreter/lexer.cpp
        src/Engine/Interpreter/lexer.h
        src/Engine/Interpreter/parser.cpp
        src/Engine/Interpreter/parser.h
        src/Engine/Interpreter/pginterpreter.cpp
        src/Engine/Interpreter/pginterpreter.h
        src/Engine/Interpreter/prettyprinter.h
        src/Engine/Interpreter/resolver.cpp
        src/Engine/Interpreter/resolver.h
        src/Engine/Interpreter/scriptcallable.h
        src/Engine/Interpreter/statement.cpp
        src/Engine/Interpreter/statement.h
        src/Engine/Interpreter/systemfunction.cpp
        src/Engine/Interpreter/systemfunction.h
        src/Engine/Interpreter/token.cpp
        src/Engine/Interpreter/token.h
        src/Engine/Interpreter/valuable.cpp
        src/Engine/Interpreter/valuable.h

        src/Engine/Loaders/fontloader.cpp
        src/Engine/Loaders/fontloader.h
        src/Engine/Loaders/stb_image.h
        src/Engine/Loaders/tileloader.cpp
        src/Engine/Loaders/tileloader.h

        src/Engine/Maths/noise.cpp
        src/Engine/Maths/noise.h
        src/Engine/Maths/pathfinding.h
        src/Engine/Maths/randomnumbergenerator.cpp
        src/Engine/Maths/randomnumbergenerator.h

        src/Engine/Memory/blockingconcurrentqueue.h
        src/Engine/Memory/concurrentqueue.h
        src/Engine/Memory/elementtype.cpp
        src/Engine/Memory/elementtype.h
        src/Engine/Memory/lightweightsemaphore.h
        src/Engine/Memory/memorypool.h
        src/Engine/Memory/parallelfor.cpp
        src/Engine/Memory/parallelfor.h
        src/Engine/Memory/rwlock.h
        src/Engine/Memory/threadpool.h

        src/Engine/Renderer/mesh.cpp
        src/Engine/Renderer/mesh.h
        src/Engine/Renderer/particle.cpp
        src/Engine/Renderer/particle.h
        src/Engine/Renderer/renderer.cpp
        src/Engine/Renderer/renderer.h
        src/Engine/Renderer/renderermodule.h

        src/Engine/Scene/sceneloader.cpp
        src/Engine/Scene/sceneloader.h
        src/Engine/Scene/scenemanager.cpp
        src/Engine/Scene/scenemanager.h

        src/Engine/Shader/shader.cpp
        src/Engine/Shader/shader.h

        src/Engine/Systems/coremodule.h
        src/Engine/Systems/coresystems.h
        src/Engine/Systems/logmodule.h
        src/Engine/Systems/scenemodule.h
        src/Engine/Systems/sentencemodule.h
        src/Engine/Systems/shape2Dmodule.h
        src/Engine/Systems/texture2Dmodule.h
        src/Engine/Systems/timemodule.h
        src/Engine/Systems/uimodule.cpp
        src/Engine/Systems/uimodule.h

        src/Engine/UI/button.cpp
        src/Engine/UI/button.h
        src/Engine/UI/focusable.cpp
        src/Engine/UI/focusable.h
        src/Engine/UI/listview.cpp
        src/Engine/UI/listview.h
        src/Engine/UI/scrollable.cpp
        src/Engine/UI/scrollable.h
        src/Engine/UI/sentencesystem.cpp
        src/Engine/UI/sentencesystem.h
        src/Engine/UI/textinput.cpp
        src/Engine/UI/textinput.h
        src/Engine/UI/uianimation.cpp
        src/Engine/UI/uianimation.h
        src/Engine/UI/uiconstant.cpp
        src/Engine/UI/uiconstant.h
        src/Engine/UI/uisystem.cpp
        src/Engine/UI/uisystem.h
)

target_include_directories(PgEngine PRIVATE src/Engine)
target_include_directories(PgEngine PRIVATE src/GameElements)

if(TARGET SDL2::SDL2main)
    target_link_libraries(PgEngine PRIVATE SDL2::SDL2main)
endif()

# Todo fix ttf doens't work

# target_link_libraries(PgEngine PRIVATE SDL2::SDL2-static SDL2_ttf::SDL2_ttf-static SDL2_mixer::SDL2_mixer-static glm)
target_link_options(PgEngine PRIVATE -static-libgcc -static-libstdc++)

if (EMSCRIPTEN)
    set_target_properties(PgEngine
        PROPERTIES SUFFIX ".html"
        LINK_FLAGS " -s USE_FREETYPE=1 -sGL_ENABLE_GET_PROC_ADDRESS -s DISABLE_EXCEPTION_CATCHING=0 -s DEMANGLE_SUPPORT=1 -s SAFE_HEAP=1 --bind -s WASM=1 -O3 -s LEGACY_GL_EMULATION=0 -s GL_UNSAFE_OPTS=0 -s ASSERTIONS=1 -s GL_ASSERTIONS=1 -s INVOKE_RUN=0 -std=c++17 -s USE_WEBGL2=1 -s FULL_ES3=1 -s USE_GLFW=3 -s OFFSCREENCANVAS_SUPPORT=1")

        #--bind -m32 -s USE_FREETYPE=1 -s USE_WEBGL2=1 -s WASM=0 -s MIN_WEBGL_VERSION=1 -s ABORT_ON_WASM_EXCEPTIONS=1 -g2 -s USE_GLFW=3 -sSAFE_HEAP")
    
    target_link_libraries(PgEngine PRIVATE SDL2::SDL2-static SDL2_mixer::SDL2_mixer-static glm)
else()
    target_link_libraries(PgEngine PRIVATE SDL2::SDL2-static SDL2_mixer::SDL2_mixer-static glm libglew_static)
endif()

#target_link_libraries(PgEngine SDL2main SDL2-static SDL2_mixer-static SDL2_ttf-static glm)

target_sources(TetrisClone PRIVATE
        src/Engine/configuration.h
        src/Engine/configuration.cpp
        src/Engine/constant.h
        src/Engine/gl_debug.hpp
        src/Engine/logger.h
        src/Engine/logger.cpp
        src/Engine/serialization.cpp
        src/Engine/serialization.h
        src/Engine/window.cpp
        src/Engine/window.h

        src/Engine/2D/simple2dobject.cpp
        src/Engine/2D/simple2dobject.h
        src/Engine/2D/texture.cpp
        src/Engine/2D/texture.h

        src/Engine/Audio/audiomodule.h
        src/Engine/Audio/audiosystem.cpp
        src/Engine/Audio/audiosystem.h

        src/Engine/ECS/callable.h
        src/Engine/ECS/commanddispatcher.cpp
        src/Engine/ECS/commanddispatcher.h
        src/Engine/ECS/componentregistry.cpp
        src/Engine/ECS/componentregistry.h
        src/Engine/ECS/ecsmodule.h
        src/Engine/ECS/entity.cpp
        src/Engine/ECS/entity.h
        src/Engine/ECS/entitycreationsystem.h
        src/Engine/ECS/entitysystem.cpp
        src/Engine/ECS/entitysystem.h
        src/Engine/ECS/eventlistener.h
        src/Engine/ECS/group.cpp
        src/Engine/ECS/group.h
        src/Engine/ECS/identifiers.h
        src/Engine/ECS/loggersystem.h
        src/Engine/ECS/sparseset.cpp
        src/Engine/ECS/sparseset.h
        src/Engine/ECS/system.cpp
        src/Engine/ECS/system.h
        src/Engine/ECS/uniqueid.cpp
        src/Engine/ECS/uniqueid.h

        src/Engine/Event/eventloop.cpp
        src/Engine/Event/eventloop.h

        src/Engine/Files/filemanager.cpp
        src/Engine/Files/filemanager.h
        src/Engine/Files/fileparser.cpp
        src/Engine/Files/fileparser.h

        src/Engine/Helpers/date.h
        src/Engine/Helpers/detector.h
        src/Engine/Helpers/openglobject.cpp
        src/Engine/Helpers/openglobject.h

        src/Engine/Input/input.cpp
        src/Engine/Input/input.h
        src/Engine/Input/inputcomponent.cpp
        src/Engine/Input/inputcomponent.h
        src/Engine/Input/inputmodule.h

        src/Engine/Interpreter/environment.cpp
        src/Engine/Interpreter/environment.h
        src/Engine/Interpreter/expression.cpp
        src/Engine/Interpreter/expression.h
        src/Engine/Interpreter/interpreter.cpp
        src/Engine/Interpreter/interpreter.h
        src/Engine/Interpreter/interpretersystem.cpp
        src/Engine/Interpreter/interpretersystem.h
        src/Engine/Interpreter/lexer.cpp
        src/Engine/Interpreter/lexer.h
        src/Engine/Interpreter/parser.cpp
        src/Engine/Interpreter/parser.h
        src/Engine/Interpreter/pginterpreter.cpp
        src/Engine/Interpreter/pginterpreter.h
        src/Engine/Interpreter/prettyprinter.h
        src/Engine/Interpreter/resolver.cpp
        src/Engine/Interpreter/resolver.h
        src/Engine/Interpreter/scriptcallable.h
        src/Engine/Interpreter/statement.cpp
        src/Engine/Interpreter/statement.h
        src/Engine/Interpreter/systemfunction.cpp
        src/Engine/Interpreter/systemfunction.h
        src/Engine/Interpreter/token.cpp
        src/Engine/Interpreter/token.h
        src/Engine/Interpreter/valuable.cpp
        src/Engine/Interpreter/valuable.h

        src/Engine/Loaders/fontloader.cpp
        src/Engine/Loaders/fontloader.h
        src/Engine/Loaders/stb_image.h
        src/Engine/Loaders/tileloader.cpp
        src/Engine/Loaders/tileloader.h

        src/Engine/Maths/noise.cpp
        src/Engine/Maths/noise.h
        src/Engine/Maths/pathfinding.h
        src/Engine/Maths/randomnumbergenerator.cpp
        src/Engine/Maths/randomnumbergenerator.h

        src/Engine/Memory/blockingconcurrentqueue.h
        src/Engine/Memory/concurrentqueue.h
        src/Engine/Memory/elementtype.cpp
        src/Engine/Memory/elementtype.h
        src/Engine/Memory/lightweightsemaphore.h
        src/Engine/Memory/memorypool.h
        src/Engine/Memory/parallelfor.cpp
        src/Engine/Memory/parallelfor.h
        src/Engine/Memory/rwlock.h
        src/Engine/Memory/threadpool.h

        src/Engine/Renderer/mesh.cpp
        src/Engine/Renderer/mesh.h
        src/Engine/Renderer/particle.cpp
        src/Engine/Renderer/particle.h
        src/Engine/Renderer/renderer.cpp
        src/Engine/Renderer/renderer.h
        src/Engine/Renderer/renderermodule.h

        src/Engine/Scene/sceneloader.cpp
        src/Engine/Scene/sceneloader.h
        src/Engine/Scene/scenemanager.cpp
        src/Engine/Scene/scenemanager.h

        src/Engine/Shader/shader.cpp
        src/Engine/Shader/shader.h

        src/Engine/Systems/coremodule.h
        src/Engine/Systems/coresystems.h
        src/Engine/Systems/logmodule.h
        src/Engine/Systems/scenemodule.h
        src/Engine/Systems/sentencemodule.h
        src/Engine/Systems/shape2Dmodule.h
        src/Engine/Systems/texture2Dmodule.h
        src/Engine/Systems/timemodule.h
        src/Engine/Systems/uimodule.cpp
        src/Engine/Systems/uimodule.h

        src/Engine/UI/button.cpp
        src/Engine/UI/button.h
        src/Engine/UI/focusable.cpp
        src/Engine/UI/focusable.h
        src/Engine/UI/listview.cpp
        src/Engine/UI/listview.h
        src/Engine/UI/scrollable.cpp
        src/Engine/UI/scrollable.h
        src/Engine/UI/sentencesystem.cpp
        src/Engine/UI/sentencesystem.h
        src/Engine/UI/textinput.cpp
        src/Engine/UI/textinput.h
        src/Engine/UI/uianimation.cpp
        src/Engine/UI/uianimation.h
        src/Engine/UI/uiconstant.cpp
        src/Engine/UI/uiconstant.h
        src/Engine/UI/uisystem.cpp
        src/Engine/UI/uisystem.h

        src/GameElements/Systems/basicsystems.h

        src/Tetris/app.h
        src/Tetris/app.cpp
        src/Tetris/tetromino.h
        src/Tetris/tetromino.cpp
)

target_include_directories(TetrisClone PRIVATE src/Engine)
target_include_directories(TetrisClone PRIVATE src/GameElements)
target_include_directories(TetrisClone PRIVATE src/Tetris)

if(TARGET SDL2::SDL2main)
    target_link_libraries(TetrisClone PRIVATE SDL2::SDL2main)
endif()

# Todo fix ttf doens't work

# target_link_libraries(PgEngine PRIVATE SDL2::SDL2-static SDL2_ttf::SDL2_ttf-static SDL2_mixer::SDL2_mixer-static glm)
target_link_options(TetrisClone PRIVATE -static-libgcc -static-libstdc++)

if (EMSCRIPTEN)
    set_target_properties(TetrisClone
        PROPERTIES SUFFIX ".html"
        LINK_FLAGS " -s USE_FREETYPE=1 -sGL_ENABLE_GET_PROC_ADDRESS -s DISABLE_EXCEPTION_CATCHING=0 -s DEMANGLE_SUPPORT=1 -s SAFE_HEAP=1 --bind -s WASM=1 -O3 -s LEGACY_GL_EMULATION=0 -s GL_UNSAFE_OPTS=0 -s ASSERTIONS=1 -s GL_ASSERTIONS=1 -s INVOKE_RUN=0 -std=c++17 -s USE_WEBGL2=1 -s FULL_ES3=1 -s USE_GLFW=3 -s OFFSCREENCANVAS_SUPPORT=1")

        #--bind -m32 -s USE_FREETYPE=1 -s USE_WEBGL2=1 -s WASM=0 -s MIN_WEBGL_VERSION=1 -s ABORT_ON_WASM_EXCEPTIONS=1 -g2 -s USE_GLFW=3 -sSAFE_HEAP")
    
    target_link_libraries(TetrisClone PRIVATE SDL2::SDL2-static SDL2_mixer::SDL2_mixer-static glm)
else()
    target_link_libraries(TetrisClone PRIVATE SDL2::SDL2-static SDL2_mixer::SDL2_mixer-static glm libglew_static)
endif()
