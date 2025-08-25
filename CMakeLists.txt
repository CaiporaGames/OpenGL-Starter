cmake_minimum_required(VERSION 3.24)
project(glfw_no_api LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad  CONFIG REQUIRED)
find_package(glm   CONFIG REQUIRED)

# Library with your app code (no main)
add_library(app_core STATIC
  src/app/App.cpp
  src/gfx/Shader.cpp
  src/gfx/TriangleRenderer.cpp
  src/gfx/SpriteBatch.cpp
  src/gfx/Texture2D.cpp
  src/thirdparty/stb_image.cpp
  src/game/Game.cpp
)

target_include_directories(app_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(app_core PUBLIC glm::glm glfw glad::glad)

if (MSVC)
  target_compile_options(app_core PUBLIC /W4 /permissive-)
endif()

# Executable with only a tiny main that spins up the App
add_executable(glfw_no_api src/main.cpp)
target_link_libraries(glfw_no_api PRIVATE app_core)

# Copy shaders next to the exe so relative paths work in VS & CLI
add_custom_command(TARGET glfw_no_api POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/shaders
          $<TARGET_FILE_DIR:glfw_no_api>/shaders)

# Copy assets next to the exe (Debug/Release)
add_custom_command(TARGET glfw_no_api POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/assets
          $<TARGET_FILE_DIR:glfw_no_api>/assets)
