find_package(GTest CONFIG REQUIRED)

add_executable(HazelTests
    HazelTestMain.cpp
    SceneTests.cpp
    TransformTests.cpp
    MaterialTests.cpp
    LightTests.cpp
    MeshImportTests.cpp
    Physics2DTests.cpp
    NativeScriptTests.cpp
    CSharpScriptTests.cpp
    AudioTests.cpp
    AnimationTests.cpp
    RuntimeTests.cpp
    HierarchyTests.cpp
)

target_compile_definitions(HazelTests PRIVATE "HAZEL_REPO_ROOT=\"${CMAKE_SOURCE_DIR}\"")

target_link_libraries(HazelTests PRIVATE Hazel::Engine GTest::gtest)
target_compile_features(HazelTests PRIVATE cxx_std_17)
hazel_set_output_dirs(HazelTests NAME Tests)

include(GoogleTest)
gtest_discover_tests(HazelTests DISCOVERY_TIMEOUT 60)
