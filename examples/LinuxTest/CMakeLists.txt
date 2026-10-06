
cmake_minimum_required(VERSION 3.25)

set (PROJECT_NAME "LinuxTest")

set (APP_INCLUDES )

file(GLOB APP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
file(GLOB APP_HEADERS)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/shaders/compiled)

add_executable(${PROJECT_NAME} ${APP_SOURCES} ${APP_HEADERS})

target_include_directories (${PROJECT_NAME} PRIVATE ${APP_INCLUDES})
target_link_libraries(${PROJECT_NAME} PRIVATE Core)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}
        PREFIX "Header Files"
        FILES ${APP_HEADERS}
    )

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}
        PREFIX "Source Files"
        FILES ${APP_SOURCES}
    )

add_custom_command(
TARGET ${PROJECT_NAME}    
POST_BUILD
COMMAND  glslc --target-env=vulkan1.3 --target-spv=spv1.3 "${CMAKE_CURRENT_SOURCE_DIR}/shaders/basic.frag" -o "${CMAKE_CURRENT_SOURCE_DIR}/shaders/compiled/basic.frag.spv"
COMMENT "Compiled basic frag"
)

add_custom_command(
TARGET ${PROJECT_NAME}    
POST_BUILD
COMMAND  glslc --target-env=vulkan1.3 --target-spv=spv1.3 "${CMAKE_CURRENT_SOURCE_DIR}/shaders/basic.vert" -o "${CMAKE_CURRENT_SOURCE_DIR}/shaders/compiled/basic.vert.spv"
COMMENT "Compiled basic vert"
)

add_custom_command(
TARGET ${PROJECT_NAME}    
POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/attachmentdescriptions/" "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}"
COMMENT "Copied Attachment Descriptions"
)

add_custom_command(
TARGET ${PROJECT_NAME}    
POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/pipelinedescriptions/" "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}"
COMMENT "Copied Pipeline Descriptions"
)

add_custom_command(
TARGET ${PROJECT_NAME}    
POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/shaders/layouts/" "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}"
COMMENT "Copied Shader Graphs Descriptions"
)

add_custom_command(
TARGET ${PROJECT_NAME}    
POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/shaders/compiled/" "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}"
COMMENT "Copied Shader Graphs Descriptions"
)