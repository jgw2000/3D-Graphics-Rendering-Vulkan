find_package(glfw3 REQUIRED)
find_package(spdlog REQUIRED)
find_package(assimp REQUIRED)
find_package(stb REQUIRED)
find_package(imgui REQUIRED)
find_package(Ktx REQUIRED)
find_package(meshoptimizer REQUIRED)

file(GLOB_RECURSE SRC_FILES source/*.c?? third_party/*.c??)
file(GLOB_RECURSE HEADER_FILES source/*.h source/*.hpp third_party/*.h)

add_library(Engine ${SRC_FILES} ${HEADER_FILES})

target_include_directories(Engine PUBLIC
    source/runtime/application
    source/runtime/render
    source/runtime/render/vma
    source/runtime/foundation
    third_party/implot
)

target_link_libraries(Engine glfw)
target_link_libraries(Engine spdlog::spdlog)
target_link_libraries(Engine assimp::assimp)
target_link_libraries(Engine stb::stb)
target_link_libraries(Engine imgui::imgui)
target_link_libraries(Engine KTX::ktx)
target_link_libraries(Engine meshoptimizer::meshoptimizer)

source_group(TREE ${PROJECT_SOURCE_DIR}/engine FILES ${SRC_FILES} ${HEADER_FILES})