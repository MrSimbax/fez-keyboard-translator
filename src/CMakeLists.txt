set(PROJECT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}/include")

if(EMSCRIPTEN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

add_executable("${CMAKE_PROJECT_NAME}"
    "main.cpp")
target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE raylib raygui)
target_include_directories("${CMAKE_PROJECT_NAME}" PRIVATE ${PROJECT_INCLUDE_DIRECTORIES})

if(MSVC)
    target_link_options("${CMAKE_PROJECT_NAME}" PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup)
else()
    target_compile_options("${CMAKE_PROJECT_NAME}" PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(EMSCRIPTEN)
    target_link_options("${CMAKE_PROJECT_NAME}" PRIVATE --shell-file "${CMAKE_CURRENT_LIST_DIR}/shell_minimal.html")
endif()

install(TARGETS "${CMAKE_PROJECT_NAME}" RUNTIME)
