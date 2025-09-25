cmake_minimum_required(VERSION 3.10)
project(GameTrainerApp)

set(CMAKE_CXX_STANDARD 17)
set(GAME_TRAINER_VERSION "0.1.0")
add_definitions(-DGAME_TRAINER_VERSION="${GAME_TRAINER_VERSION}")

# Find OpenCV
find_package(OpenCV REQUIRED)

# Include directories
include_directories(${OpenCV_INCLUDE_DIRS})

# Source files
set(SOURCES
    src/main.cpp
    src/ScreenCapture.cpp
    src/InputTracker.cpp
    src/EventLogger.cpp
    src/SessionManager.cpp
    src/ConcentrationTracker.cpp
    src/ReviewInterface.cpp
    src/GameplayAnalyzer.cpp
    src/EnemyDetector.cpp
    src/CombatAnalyzer.cpp
    src/VideoRecorder.cpp
    src/PositionTracker.cpp
)

add_executable(GameTrainerApp ${SOURCES})

# Link OpenCV libraries
target_link_libraries(GameTrainerApp ${OpenCV_LIBS})