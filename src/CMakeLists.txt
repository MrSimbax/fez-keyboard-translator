set(PROJECT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}/include")

if(EMSCRIPTEN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

add_executable(main
    "main.cpp")
target_link_libraries(main PRIVATE raylib raygui)
target_include_directories(main PRIVATE ${PROJECT_INCLUDE_DIRECTORIES})
target_compile_options(main PRIVATE "-Wall;-Wextra;-Wpedantic")
install(TARGETS main RUNTIME)
