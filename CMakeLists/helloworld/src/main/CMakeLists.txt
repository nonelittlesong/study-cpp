aux_source_directory(. srcs)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_executable(helloShared ${srcs})

link_directories(${PROJECT_SOURCE_DIR}/lib)

target_link_libraries(helloShared
                      sharedd)