cmake_minimum_required(VERSION 3.24)
project(glfw_no_api LANGUAGES C CXX)

option(ENABLE_TESTS "Build unit tests" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------- vcpkg packages ----------------
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)   # provides imgui::imgui (with backends if features enabled)
find_package(glad CONFIG REQUIRED)


# ---------------- core (GL-free) ----------------
add_library(core STATIC)
target_sources(core PRIVATE
  core/src/io/obj_loader.cpp
)
target_sources(core PUBLIC
  FILE_SET HEADERS
  BASE_DIRS core/include
  FILES
    core/include/core/aabb.hpp
    core/include/core/camera/OrbitCamera3D.hpp
    core/include/core/ray.hpp
    core/include/core/raycast.hpp
    core/include/core/camera/ScreenRay.hpp
    core/include/core/frustum.hpp
    core/include/core/bvh.hpp
    core/include/core/mesh_data.hpp
    core/include/core/io/obj_loader.hpp
)
target_include_directories(core PUBLIC core/include)
target_link_libraries(core PUBLIC glm::glm tinyobjloader::tinyobjloader)

# ---------------- app_core (GL/UI) --------------
add_library(app_core STATIC
  src/app/App.cpp
  src/gfx/Shader.cpp
  src/gfx/TriangleRenderer.cpp
  src/gfx/SpriteBatch.cpp
  src/gfx/Texture2D.cpp
  src/thirdparty/stb_image.cpp
  src/game/PongScene.cpp

  include/gfx/GLDebug.hpp
  src/gfx/GLDebug.cpp

  include/render/Mesh3D.hpp
  src/render/Mesh3D.cpp
  include/render/DebugLines3D.hpp
  src/render/DebugLines3D.cpp
  include/render/DebugCross3D.hpp
  src/render/DebugCross3D.cpp
  include/render/BoxWire3D.hpp
  src/render/BoxWire3D.cpp
  include/render/MeshGL.hpp
  src/render/MeshGL.cpp

  include/ui/ImGuiLayer.hpp
  src/ui/ImGuiLayer.cpp
  include/util/Screenshot.hpp
  src/util/Screenshot.cpp

  shaders/basic3d.vert
  shaders/basic3d.frag
)
target_include_directories(app_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/core/include
)
# Let ImGui backends know we use GLAD
target_compile_definitions(app_core PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)

# ---- ImGui backends handling ----
# If you vendor the two backend .cpp files, we compile them.
# Otherwise (default), we just link the vcpkg imgui::imgui which already includes backends.
set(IMGUI_VENDOR_BACKENDS_DIR "${CMAKE_SOURCE_DIR}/thirdparty/imgui/backends")
if (EXISTS "${IMGUI_VENDOR_BACKENDS_DIR}/imgui_impl_glfw.cpp" AND
    EXISTS "${IMGUI_VENDOR_BACKENDS_DIR}/imgui_impl_opengl3.cpp")
  add_library(imgui_backends STATIC
    ${IMGUI_VENDOR_BACKENDS_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_VENDOR_BACKENDS_DIR}/imgui_impl_opengl3.cpp
  )
  target_include_directories(imgui_backends PUBLIC ${IMGUI_VENDOR_BACKENDS_DIR})
  target_compile_definitions(imgui_backends PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)
  target_link_libraries(imgui_backends PUBLIC imgui::imgui glfw glad::glad)
  target_link_libraries(app_core PUBLIC imgui_backends)
endif()

target_link_libraries(app_core
  PUBLIC  glm::glm glfw glad::glad tinyobjloader::tinyobjloader core
  PRIVATE $<$<TARGET_EXISTS:imgui_backends>:imgui_backends> imgui::imgui
)
# ---------------- exe ---------------------------
add_executable(glfw_no_api src/main.cpp)
target_link_libraries(glfw_no_api PRIVATE app_core core)

# Copy runtime assets
add_custom_command(TARGET glfw_no_api POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/shaders
          $<TARGET_FILE_DIR:glfw_no_api>/shaders)
add_custom_command(TARGET glfw_no_api POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/assets
          $<TARGET_FILE_DIR:glfw_no_api>/assets)

# ---------------- tests (opt) -------------------
if (ENABLE_TESTS)
  enable_testing()
  find_package(doctest CONFIG REQUIRED)
  add_executable(core_tests tests/test_aabb.cpp)
  target_sources(core_tests PRIVATE
    tests/test_orbit_cam.cpp
    tests/test_bvh.cpp
  )
  target_link_libraries(core_tests PRIVATE core doctest::doctest)
  target_include_directories(core_tests PRIVATE core/include)
  add_test(NAME core_tests COMMAND core_tests)
endif()

# -------- install targets (runtime layout) --------
install(TARGETS glfw_no_api RUNTIME DESTINATION .)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets  DESTINATION .)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/shaders DESTINATION .)

# -------- CPack zip (optional local packaging) --------
set(CPACK_PACKAGE_NAME "glfw-no-api")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_GENERATOR "ZIP")
include(CPack)
