file(GLOB_RECURSE SRC_FILES source/*.c??)
file(GLOB_RECURSE HEADER_FILES source/*.h)
file(GLOB_RECURSE EXTERNAL_LIB third_party/lib/*.lib)

add_library(Engine ${SRC_FILES} ${HEADER_FILES})

target_include_directories(Engine PUBLIC
    source/runtime/application
    source/runtime/render
    source/runtime/foundation
    third_party/include
)

target_link_libraries(Engine ${EXTERNAL_LIB})

source_group(TREE ${PROJECT_SOURCE_DIR}/engine FILES ${SRC_FILES} ${HEADER_FILES})