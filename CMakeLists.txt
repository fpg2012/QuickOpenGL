# CMakeList.txt : CMake project for QuickOpenGL, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("QuickOpenGL")

add_subdirectory("glfw")
include_directories("glad/include")
set(GLAD_SRC "glad/src/glad.c")

set(MY_HEADERS shader.hpp utils.hpp shape.hpp config.h utils.hpp camera.hpp material.hpp light.hpp texture.hpp gltf_scene.hpp)
set(TINY_GLTF_SRC tiny_gltf.h json.hpp)
set(STB_SRC stb_image_write.h stb_image.h)

add_subdirectory("glm")

# Add source to this project's executable.
add_executable (QuickOpenGL "QuickOpenGL.cpp" ${MY_HEADERS} ${GLAD_SRC} ${STB_SRC} ${TINY_GLTF_SRC})
target_link_libraries(QuickOpenGL glfw glm::glm-header-only)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET QuickOpenGL PROPERTY CXX_STANDARD 20)
endif()

file(GLOB shaders ${CMAKE_SOURCE_DIR}/*.vert
                      ${CMAKE_SOURCE_DIR}/*.frag)

add_custom_target(CopyShaders)
foreach(shader ${shaders})
  add_custom_command(TARGET CopyShaders PRE_BUILD DEPENDS ${shader}
                     COMMAND ${CMAKE_COMMAND} -E
                     copy ${shader} $<TARGET_FILE_DIR:QuickOpenGL>)
endforeach()

add_custom_target(all_target DEPENDS QuickOpenGL, CopyShaders)