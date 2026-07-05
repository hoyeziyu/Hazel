include(FetchContent)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
)
FetchContent_MakeAvailable(googlebenchmark)

add_executable(HazelBench SceneBench.cpp main.cpp)

target_link_libraries(HazelBench PRIVATE
    benchmark::benchmark
    benchmark::benchmark_main
    Hazel::Engine
)
target_compile_features(HazelBench PRIVATE cxx_std_17)
hazel_set_output_dirs(HazelBench NAME Benchmarks)
