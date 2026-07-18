# Vendor ImGui (ocornut docking branch) + ImGuizmo — replaces vcpkg imgui/imguizmo

set(HAZEL_IMGUI_DIR "${CMAKE_SOURCE_DIR}/vendor/imgui")
set(HAZEL_IMGUIZMO_DIR "${CMAKE_SOURCE_DIR}/vendor/ImGuizmo")

add_library(imgui STATIC
    "${HAZEL_IMGUI_DIR}/imgui.cpp"
    "${HAZEL_IMGUI_DIR}/imgui_draw.cpp"
    "${HAZEL_IMGUI_DIR}/imgui_tables.cpp"
    "${HAZEL_IMGUI_DIR}/imgui_widgets.cpp"
    "${HAZEL_IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp"
    "${HAZEL_IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${HAZEL_IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)

target_include_directories(imgui PUBLIC
    "${HAZEL_IMGUI_DIR}"
    "${HAZEL_IMGUI_DIR}/backends"
)

target_compile_definitions(imgui PUBLIC IMGUI_DEFINE_MATH_OPERATORS)
target_link_libraries(imgui PUBLIC glfw glad::glad)

add_library(imgui::imgui ALIAS imgui)

add_library(imguizmo STATIC
    "${HAZEL_IMGUIZMO_DIR}/src/ImGuizmo.cpp"
)

target_include_directories(imguizmo PUBLIC "${HAZEL_IMGUIZMO_DIR}/src")
target_link_libraries(imguizmo PUBLIC imgui)

add_library(imguizmo::imguizmo ALIAS imguizmo)
