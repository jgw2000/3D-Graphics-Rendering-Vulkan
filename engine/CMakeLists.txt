find_package(glfw3 REQUIRED)
find_package(spdlog REQUIRED)
find_package(assimp REQUIRED)
find_package(stb REQUIRED)

file(GLOB_RECURSE SRC_FILES source/*.c??)
file(GLOB_RECURSE HEADER_FILES source/*.h source/*.hpp)

add_library(Engine ${SRC_FILES} ${HEADER_FILES})

target_include_directories(Engine PUBLIC
    source/runtime/application
    source/runtime/render
    source/runtime/render/vma
    source/runtime/foundation
)

target_link_libraries(Engine glfw)
target_link_libraries(Engine spdlog::spdlog)
target_link_libraries(Engine assimp::assimp)
target_link_libraries(Engine stb::stb)

source_group(TREE ${PROJECT_SOURCE_DIR}/engine FILES ${SRC_FILES} ${HEADER_FILES})