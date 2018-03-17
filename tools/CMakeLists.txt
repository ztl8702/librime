set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

if (NOT EMSCRIPTEN)
set(rime_api_console_src "rime_api_console.cc")
add_executable(rime_api_console ${rime_api_console_src})
target_link_libraries(rime_api_console ${rime_library} ${rime_gears_library})
add_dependencies(rime_api_console ${rime_library} ${rime_gears_library})

set(rime_patch_src "rime_patch.cc")
add_executable(rime_patch ${rime_patch_src})
target_link_libraries(rime_patch ${rime_library} ${rime_gears_library})
add_dependencies(rime_patch ${rime_library} ${rime_gears_library})
endif()

# msvc doesn't export all symbols
if(NOT (WIN32 AND BUILD_SHARED_LIBS))

set(rime_console_src "rime_console.cc")
add_executable(rime_console ${rime_console_src})
target_link_libraries(rime_console ${rime_library} ${rime_gears_library})
add_dependencies(rime_console ${rime_library} ${rime_gears_library})
if (EMSCRIPTEN)
set_target_properties(
    rime_console 
    PROPERTIES 
        SUFFIX ".html"
)
message("==========CURRENT FLAGS")
message(${CMAKE_CXX_FLAGS})
#target_compile_options(rime_console PUBLIC "--preload-file ../build/bin@data")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ALLOW_MEMORY_GROWTH=1 -s TOTAL_MEMORY=256MB --preload-file ${CMAKE_CURRENT_LIST_DIR}/../tdata@data")
message("==========CURRENT FLAGS")
message(${CMAKE_CXX_FLAGS})
endif()

if (NOT EMSCRIPTEN)
set(rime_dict_manager_src "rime_dict_manager.cc")
add_executable(rime_dict_manager ${rime_dict_manager_src})
target_link_libraries(rime_dict_manager ${rime_library} ${rime_gears_library})
add_dependencies(rime_dict_manager ${rime_library} ${rime_gears_library})

set(rime_deployer_src "rime_deployer.cc")
add_executable(rime_deployer ${rime_deployer_src})
target_link_libraries(rime_deployer ${rime_library} ${rime_gears_library})
add_dependencies(rime_deployer ${rime_library} ${rime_gears_library})

install(TARGETS rime_dict_manager DESTINATION ${BIN_INSTALL_DIR})
install(TARGETS rime_deployer DESTINATION ${BIN_INSTALL_DIR})
endif(NOT EMSCRIPTEN)
endif()

file(COPY ${PROJECT_SOURCE_DIR}/data/minimal/default.yaml
     DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY ${PROJECT_SOURCE_DIR}/data/minimal/symbols.yaml
     DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY ${PROJECT_SOURCE_DIR}/data/minimal/essay.txt
     DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY ${PROJECT_SOURCE_DIR}/data/minimal/luna_pinyin.dict.yaml
     DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY ${PROJECT_SOURCE_DIR}/data/minimal/luna_pinyin.schema.yaml
     DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY ${PROJECT_SOURCE_DIR}/data/minimal/cangjie5.dict.yaml
     DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY ${PROJECT_SOURCE_DIR}/data/minimal/cangjie5.schema.yaml
     DESTINATION ${EXECUTABLE_OUTPUT_PATH})
