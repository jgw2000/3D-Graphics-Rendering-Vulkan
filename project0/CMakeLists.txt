file(GLOB_RECURSE SRC_FILES *.c??)
file(GLOB_RECURSE HEADER_FILES *.h)

add_executable(Project0 ${SRC_FILES} ${HEADER_FILES})
target_link_libraries(Project0 PUBLIC Engine)

source_group(TREE ${PROJECT_SOURCE_DIR}/project0 FILES ${SRC_FILES} ${HEADER_FILES})
set_property(TARGET Project0 PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/Project0")