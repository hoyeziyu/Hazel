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
)

target_link_libraries(HazelTests PRIVATE Hazel::Engine GTest::gtest)
target_compile_features(HazelTests PRIVATE cxx_std_17)
hazel_set_output_dirs(HazelTests NAME Tests)

include(GoogleTest)
gtest_discover_tests(HazelTests)
