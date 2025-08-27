cmake_minimum_required(VERSION 3.24)
project(glfw_no_api LANGUAGES C CXX)


if (ENABLE_TESTS)
  find_package(doctest CONFIG REQUIRED)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# still use vcpkg glfw + glm
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

# our custom glad (Glad 1.x from thirdparty/)
add_library(glad STATIC ${CMAKE_CURRENT_SOURCE_DIR}/include/thirdparty/glad/src/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/thirdparty/glad/include)


# ---- app_core ----
add_library(app_core STATIC
  src/app/App.cpp
  src/gfx/Shader.cpp
  src/gfx/TriangleRenderer.cpp
  src/gfx/SpriteBatch.cpp
  src/gfx/Texture2D.cpp
  src/thirdparty/stb_image.cpp
  src/game/PongScene.cpp
  src/gfx/GLDebug.cpp
  include/gfx/GLDebug.hpp
  include/render/Mesh3D.hpp
  src/render/Mesh3D.cpp
  shaders/basic3d.vert
  shaders/basic3d.frag
)

target_include_directories(app_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/core/include
)

target_link_libraries(app_core PUBLIC glm::glm glfw glad)

# ---- core (GL-free) ----
add_library(core STATIC
  core/include/core/aabb.hpp
)

target_sources(core PUBLIC
  FILE_SET HEADERS
  BASE_DIRS core/include
  FILES
    core/include/core/camera/OrbitCamera3D.hpp
)

target_include_directories(core PUBLIC core/include)
target_link_libraries(core PUBLIC glm::glm)

if (MSVC)
  target_compile_options(app_core PUBLIC /W4 /permissive-)
endif()

# ---- exe ----
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

if (ENABLE_TESTS)
  add_executable(core_tests tests/test_aabb.cpp)
  target_sources(core_tests PRIVATE
    tests/test_orbit_cam.cpp
  )
  target_link_libraries(core_tests PRIVATE core doctest::doctest)
  target_include_directories(core_tests PRIVATE core/include)
  add_test(NAME core_tests COMMAND core_tests)
endif()
