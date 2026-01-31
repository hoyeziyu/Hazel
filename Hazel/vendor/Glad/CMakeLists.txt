cmake_minimum_required(VERSION 3.16)

# 1. 定义项目名称
project(Glad)

# 2. 将 Glad 设置为静态库，并包含源文件
# 假设你的目录结构是：
# Glad/
#  ├── CMakeLists.txt
#  ├── src/
#  │    └── glad.c
#  └── include/
#       └── glad/glad.h
add_library(Glad STATIC 
    "src/glad.c"  # 请根据你的实际路径修改这里
)

# 3. 配置包含路径
# PUBLIC 表示：编译 Glad 自身需要这个路径，链接 Glad 的主项目也需要这个路径
target_include_directories(Glad PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)