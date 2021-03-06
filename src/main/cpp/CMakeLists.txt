cmake_minimum_required(VERSION 3.4)
project(renderer)

# Functions
function(nova_set_all_target_outputs target output_dir)
    set_target_properties(${target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${output_dir})
    set_target_properties(${target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG ${output_dir})
    set_target_properties(${target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE ${output_dir})
    set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${output_dir})
    set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${output_dir})
    set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${output_dir})
    set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${output_dir})
    set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${output_dir})
    set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${output_dir})
endfunction()

# Initialize cmake options
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CONFIGURATION_TYPES Debug) # This seems to be needed to actually change

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Setup 3rd party dependencies.
set(3RD_PARTY_DIR ${CMAKE_CURRENT_LIST_DIR}/3rdparty)

find_package(OpenGL)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

find_package(JNI)

# need to compile GLFW and JSON
add_subdirectory("${3RD_PARTY_DIR}/glfw")
add_subdirectory("${3RD_PARTY_DIR}/json")
add_subdirectory("${3RD_PARTY_DIR}/glm")
add_subdirectory("${3RD_PARTY_DIR}/easyloggingpp")
add_subdirectory("${3RD_PARTY_DIR}/miniz")
add_subdirectory("${3RD_PARTY_DIR}/googletest")
add_subdirectory("${3RD_PARTY_DIR}/optional")

include_directories(SYSTEM
        ${JNI_INCLUDE_DIRS}
        "${3RD_PARTY_DIR}/glfw/include"
        "${3RD_PARTY_DIR}/easyloggingpp/src"
        "${3RD_PARTY_DIR}/glad/include"
        "${3RD_PARTY_DIR}/glm"
        "${3RD_PARTY_DIR}/miniz"
        "${3RD_PARTY_DIR}/json/src"
        "${3RD_PARTY_DIR}/googletest/googletest/include"
        "${3RD_PARTY_DIR}/optional"
        "${3RD_PARTY_DIR}/renderdocapi"
        )

set(COMMON_LINK_LIBS ${CMAKE_DL_LIBS} glfw ${OPENGL_LIBRARIES} ${JNI_LIBRARIES})

# Setup the nova-core library.
set(NOVA_HEADERS

        render/objects/uniform_buffers/uniform_buffer_definitions.h

        mc_interface/nova.h
        render/nova_renderer.h
        render/objects/textures/texture_manager.h
        utils/types.h

        render/objects/shaders/gl_shader_program.h
        render/objects/uniform_buffers/gl_uniform_buffer.h
        render/objects/gl_mesh.h
        render/objects/textures/texture2D.h

        render/windowing/glfw_gl_window.h

		input/InputHandler.h

        interfaces/iwindow.h

        mc_interface/mc_gui_objects.h
        mc_interface/mc_objects.h

        utils/utils.h
        data_loading/settings.h
        data_loading/loaders/loaders.h
        data_loading/loaders/shader_loading.h
        data_loading/loaders/loader_utils.h
        geometry_cache/mesh_store.h
        render/objects/render_object.h
        render/objects/uniform_buffers/uniform_buffer_store.h
        render/objects/uniform_buffers/uniform_buffer_definitions.h
        render/objects/uniform_buffers/gl_uniform_buffer.h

        data_loading/physics/aabb.h
        data_loading/loaders/shader_source_structs.h
        geometry_cache/mesh_definition.h
        render/objects/camera.h
        render/objects/framebuffer.h
        utils/io.h
        data_loading/direct_buffers.h
        mc_interface/nova_jni.h
        render/objects/render_object.h
        utils/profiler.h
        )

set(NOVA_SOURCE
        # Need to compile third party stuff, too...
        3rdparty/glad/src/glad.c
        3rdparty/miniz/miniz.c

        render/nova_renderer.cpp
        mc_interface/nova_facade.cpp
        render/objects/textures/texture_manager.cpp
        render/objects/uniform_buffers/uniform_buffer_store.cpp

        input/InputHandler.cpp

        render/objects/shaders/gl_shader_program.cpp
        render/objects/gl_mesh.cpp
        render/objects/textures/texture2D.cpp

        render/windowing/glfw_gl_window.cpp

        utils/utils.cpp

        data_loading/settings.cpp
        data_loading/loaders/shader_loading.cpp
        data_loading/loaders/loader_utils.cpp

        render/objects/shaders/shaderpack.cpp
        geometry_cache/mesh_store.cpp
        data_loading/physics/aabb.cpp
        geometry_cache/mesh_definition.cpp

        render/objects/uniform_buffers/uniform_buffers_definitions.cpp

        mc_interface/mc_objects.cpp
        render/objects/framebuffer.cpp
        render/objects/camera.cpp
        data_loading/loaders/shader_source_structs.cpp
        data_loading/direct_buffers.cpp
        render/objects/render_object.cpp
        utils/profiler.cpp)

if (WIN32)
    set(NOVA_SOURCE ${NOVA_SOURCE} ${NOVA_HEADERS} 3rdparty/renderdocapi/RenderDocManager.cpp utils/stb_image_write.h)
endif (WIN32)

if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DELPP_STACKTRACE_ON_CRASH -g")
endif (UNIX)

# For now just put everthing in a single nova source group
# because there are several sub groups that would be extremely small
source_group("nova" FILES ${NOVA_HEADERS} ${NOVA_NO_COMPILE} ${NOVA_SOURCE})

add_library(nova-renderer-obj OBJECT ${NOVA_SOURCE} ${GLAD_SOURCE} ${MINIZ_SOURCE})
set_target_properties(nova-renderer-obj PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

target_compile_definitions(nova-renderer-obj PRIVATE DLL_EXPORT ELPP_THREAD_SAFE)

target_compile_definitions(nova-renderer-obj PUBLIC DLL_EXPORT ELPP_THREAD_SAFE)

add_library(nova-renderer SHARED $<TARGET_OBJECTS:nova-renderer-obj>)

if (WIN32)
    set_target_properties(nova-renderer PROPERTIES PREFIX "")
endif (WIN32)

# This is needed on MSVC otherwise the build files get spread out all over the place.
# The only *problem* is that we wont be able to create multi-config builds. Personally
# I dont see this as a problem currently. This will make the location more predictable
# cross platform as well. Though i think object files are still cached across build
# types so this should be fine.
get_target_property(OUTPUT_DIR nova-renderer LIBRARY_OUTPUT_DIRECTORY)
nova_set_all_target_outputs(nova-renderer ${OUTPUT_DIR})
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set_target_properties(nova-renderer PROPERTIES CXX_VISIBILITY_PRESET hidden)
endif()

#set_target_properties(nova-renderer-obj PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

# Not sure if this is actually doing anything
if(GLFW_COMPILED)
        add_dependencies(nova-renderer glfw3)
endif()

target_link_libraries(nova-renderer ${COMMON_LINK_LIBS})

if (WIN32)
add_custom_command(TARGET nova-renderer
                   POST_BUILD
                   COMMAND copy /Y "nova-renderer.dll" "..\\..\\..\\jars\\versions\\1.10\\1.10-natives")
endif (WIN32)
if (UNIX)
add_custom_command(TARGET nova-renderer
                   POST_BUILD
                   COMMAND cp -f "${CMAKE_CURRENT_LIST_DIR}/libnova-renderer.so" "${CMAKE_CURRENT_LIST_DIR}/../../../jars/versions/1.10/1.10-natives")
endif (UNIX)

# Setup the nova-test executable
#set(TEST_SOURCE_FILES
#        3rdparty/glad/src/glad.c

#        test/main.cpp

#        test/model/loaders/shader_loading_test.cpp
#        test/render/objects/textures/texture_manager_test.cpp
#        test/render/objects/shaders/gl_shader_program_test.cpp
#        test/geometry_cache/mesh_store_test.cpp
#        test/test_utils.cpp
#        test/test_utils.h)

#source_group("test" FILES ${TEST_SOURCE_FILES})

#add_executable(nova-test ${TEST_SOURCE_FILES} ${NOVA_SOURCE})
#target_compile_definitions(nova-test PUBLIC STATIC_LINKAGE)
#target_link_libraries(nova-test gtest ${COMMON_LINK_LIBS})
#set_target_properties(nova-test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# Needed for a similar reason as the libary
#if (MSVC)
#    nova_set_all_target_outputs(nova-test "run")
#endif()
