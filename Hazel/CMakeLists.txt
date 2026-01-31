cmake_minimum_required(VERSION 3.16)

project(hazel-engine)

# 添加 vendor/spdlog 子目录
add_subdirectory(vendor/spdlog)
add_subdirectory(vendor/GLFW)
add_subdirectory(vendor/Glad)
add_subdirectory(vendor/imgui)
add_subdirectory(vendor/glm)
add_subdirectory(vendor/stb_image)
add_subdirectory(vendor/entt)
add_subdirectory(vendor/yaml-cpp)
#add_subdirectory(vendor/ImGuizmo)

list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/install")  # 没生效不知道为什么？
# 替代方案
set(GreeterLib_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/greeter/install/lib/cmake/GreeterLib")

# 打印一下路径（可选，用于在输出窗口调试，确保路径看起来是对的）
message(STATUS "Searching for GreeterLib in: ${GreeterLib_DIR}")
find_package(GreeterLib REQUIRED)
if (GreeterLib_FOUND)
  message(STATUS "Using Installed GreeterLib")
else()
  add_subdirectory(vendor/greeter)
endif()

# Add sources (only .cpp)
set (HAZEL_SOURCES
    # 源代码
    "src/hzpch.h"
    "src/hzpch.cpp"
    "src/Hazel/Core/Log.h"
    "src/Hazel/Core/Log.cpp"
    "src/Hazel/Core/Application.h"
    "src/Hazel/Core/Application.cpp"
    
    "src/Platform/Windows/WindowsWindow.h"
    "src/Platform/Windows/WindowsWindow.cpp"
    "src/Platform/Windows/WindowsInput.h"
    "src/Platform/Windows/WindowsInput.cpp"
    
    # 事件系统头文件
    "src/Hazel/Events/Event.h"
    "src/Hazel/Events/ApplicationEvent.h"
    "src/Hazel/Events/KeyEvent.h"
    "src/Hazel/Events/MouseEvent.h"
    "src/Hazel/Core/Layer.h" 
    "src/Hazel/Core/Layer.cpp" 
    "src/Hazel/Core/LayerStack.h" 
    "src/Hazel/Core/LayerStack.cpp"
    #"src/Platform/OpenGL/ImGuiOpenGLRenderer.h" 
    #"src/Platform/OpenGL/ImGuiOpenGLRenderer.cpp" 
    "src/Hazel/ImGui/ImGuiLayer.h" 
    "src/Hazel/ImGui/ImGuiLayer.cpp" 
    #"src/Platform/GLFW/ImGuiGLFWRenderer.h" 
    #"src/Platform/GLFW/ImGuiGLFWRenderer.cpp"

    "src/Hazel/Core/Input.h"
    "src/Hazel/Core/KeyCodes.h" 
    "src/Hazel/Core/MouseCodes.h"
    "src/Hazel/Core/Timestep.h" 
    "src/Hazel/Core/Core.h"
    "src/Hazel/Core/EntryPoint.h"
    "src/Hazel/Core/Window.h"
    "src/Hazel/Core/Window.cpp"

    "src/Hazel/Scene/Components.h" 
    "src/Hazel/Scene/Scene.cpp" 
    "src/Hazel/Scene/Scene.h" 

    "src/Hazel/Renderer/GraphicsContext.h" 
    "src/Platform/OpenGL/OpenGLContext.h" 
    "src/Platform/OpenGL/OpenGLContext.cpp" 
    "src/Hazel/Renderer/Shader.h" 
    "src/Hazel/Renderer/Shader.cpp" 
    "src/Hazel/Renderer/Buffer.h" 
    "src/Hazel/Renderer/Buffer.cpp" 
    "src/Hazel/Renderer/Renderer.h" 
    "src/Hazel/Renderer/Renderer.cpp" 
    "src/Hazel/Renderer/Framebuffer.h" 
    "src/Hazel/Renderer/Framebuffer.cpp" 
    "src/Platform/OpenGL/OpenGLBuffer.h" 
    "src/Platform/OpenGL/OpenGLBuffer.cpp" 
    "src/Hazel/Renderer/VertexArray.h" 
    "src/Hazel/Renderer/VertexArray.cpp" 
    "src/Platform/OpenGL/OpenGLVertexArray.h" 
    "src/Platform/OpenGL/OpenGLVertexArray.cpp" 
    "src/Hazel/Renderer/RenderCommand.h" 
    "src/Hazel/Renderer/RenderCommand.cpp" 
    "src/Hazel/Renderer/RendererAPI.h" 
    "src/Hazel/Renderer/RendererAPI.cpp" 
    "src/Platform/OpenGL/OpenGLRendererAPI.h" 
    "src/Platform/OpenGL/OpenGLRendererAPI.cpp"
    "src/Hazel/Renderer/OrthographicCamera.h" 
    "src/Hazel/Renderer/OrthographicCamera.cpp" 
    "src/Platform/OpenGL/OpenGLShader.h" 
    "src/Platform/OpenGL/OpenGLShader.cpp" 
    "src/Platform/OpenGL/OpenGLTexture.h" 
    "src/Platform/OpenGL/OpenGLTexture.cpp"
    "src/Platform/OpenGL/OpenGLFramebuffer.h" 
    "src/Platform/OpenGL/OpenGLFramebuffer.cpp"
    "src/Hazel/Renderer/Texture.h" 
    "src/Hazel/Renderer/Texture.cpp" 
    "src/Hazel/Renderer/OrthographicCameraController.h"
    "src/Hazel/Renderer/OrthographicCameraController.cpp"
    "src/Hazel/Renderer/Renderer2D.h" 
    "src/Hazel/Renderer/Renderer2D.cpp"
    "src/Hazel/Debug/Instrumentor.h" 
    "src/Hazel/Scene/Entity.h" 
    "src/Hazel/Scene/Entity.cpp" 
    "src/Hazel/Renderer/Camera.h" 
    "src/Hazel/Scene/SceneCamera.h" 
    "src/Hazel/Scene/SceneCamera.cpp" 
    "src/Hazel/Scene/ScriptableEntity.h"
    #"src/Hazel/Core/Assert.h" 
    "src/Hazel/Scene/SceneSerializer.h" 
    "src/Hazel/Scene/SceneSerializer.cpp"
    "src/Hazel/Utils/PlatformUtils.h" 
    "src/Platform/Windows/WindowsPlatformUtils.cpp" 
    #"src/Hazel/Math/Math.h" 
    #"src/Hazel/Math/Math.cpp"
)

foreach(FILE ${HAZEL_SOURCES}) 
    get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)
    string(REPLACE "/" "\\" GROUP_NAME "${PARENT_DIR}")
    source_group("${GROUP_NAME}" FILES "${FILE}")
endforeach()

# Define shared library
# Create nice looking alias for clients of Hazel to link to 
add_library(${PROJECT_NAME} ${HAZEL_SOURCES})
add_library(Hazel::Engine ALIAS ${PROJECT_NAME}) 
target_precompile_headers(${PROJECT_NAME} PRIVATE "src/hzpch.h")


if(CMAKE_SIZEOF_VOID_P EQUAL 8) 
  target_compile_definitions(${PROJECT_NAME} PRIVATE ARCH_64BIT) 
  message(STATUS "Targeting 64-bit architecture.") 
else() 
  target_compile_definitions(${PROJECT_NAME} PRIVATE ARCH_32BIT) 
  message(STATUS "Targeting 32-bit architecture.") 
endif() 


# Platform specific preprocessor defines
# PUBLIC  = define is visible to the user of Hazel
# PRIVATE = define is visible Hazel only
if (WIN32)

    target_compile_definitions(${PROJECT_NAME} PUBLIC HZ_PLATFORM_WINDOWS=1)
    target_compile_definitions(${PROJECT_NAME} PUBLIC HZ_ENABLE_ASSERTS=1)
    if(BUILD_SHARED_LIBS)
        target_compile_definitions(${PROJECT_NAME} PRIVATE HZ_BUILD_DLL=1)
    endif()
endif()

# Set include directories
target_include_directories(${PROJECT_NAME} PUBLIC 
${PROJECT_SOURCE_DIR}/src
${PROJECT_SOURCE_DIR}/vendor/GLFW/include
${PROJECT_SOURCE_DIR}/vendor/imgui
${PROJECT_SOURCE_DIR}/vendor/stb_image
${PROJECT_SOURCE_DIR}/vendor/entt/include
${PROJECT_SOURCE_DIR}/vendor/yaml-cpp/include
)

set (HAZEL_Include_Inside
    ${PROJECT_SOURCE_DIR}/src/Hazel
)
target_include_directories(${PROJECT_NAME} PRIVATE ${HAZEL_Include_Inside})

# Language standard
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

# Link libraries (e.g. logging)
target_link_libraries(${PROJECT_NAME} PUBLIC 
    spdlog
    glfw
    opengl32
    Glad
    imgui
    glm
    stb_image
    entt
    yaml-cpp
    #ImGuizmo
    Greeter::Lib
)


if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    # 将安装根目录设为项目根目录下的 dist (建议改名 dist，避免跟中间构建目录 bin 混淆)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/bins" CACHE PATH "..." FORCE)
endif()

install(TARGETS ${PROJECT_NAME} 
  DESTINATION "${CMAKE_SOURCE_DIR}/bin"
)