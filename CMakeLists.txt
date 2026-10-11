cmake_minimum_required(VERSION 3.10)
project(GameHubPinball)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenGL
find_package(OpenGL REQUIRED)

# GLFW
find_package(glfw3 REQUIRED)

# GLM
find_package(glm REQUIRED)
find_package(Threads REQUIRED)

# Include directories
include_directories(
    ${OPENGL_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/glad/include
)

# Source files
file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "include/*.h")

# Glad source
set(GLAD_SOURCES ${CMAKE_SOURCE_DIR}/external/glad/src/glad.c)

# Create executable
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/main.cpp")
add_library(pinball_core STATIC ${SOURCES} ${GLAD_SOURCES})
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE pinball_core)
include(CTest)
if(BUILD_TESTING)
    add_executable(pinball_physics_tests tests/PhysicsTests.cpp)
    target_link_libraries(pinball_physics_tests PRIVATE pinball_core)
    add_test(NAME pinball_physics COMMAND pinball_physics_tests)
    add_executable(room_tests tests/RoomTests.cpp)
    target_link_libraries(room_tests PRIVATE pinball_core)
    add_test(NAME arena_controls COMMAND room_tests)
    add_executable(raytracing_tests tests/RayTracingTests.cpp)
    target_link_libraries(raytracing_tests PRIVATE pinball_core)
    add_test(NAME raytracing COMMAND raytracing_tests)
endif()

# Link libraries
target_link_libraries(pinball_core PUBLIC
    ${OPENGL_LIBRARIES}
    glfw
    ${GLM_LIBRARIES}
    Threads::Threads
)

# Copy shaders to build directory at configure and after each build
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/shaders
            ${CMAKE_BINARY_DIR}/shaders)
