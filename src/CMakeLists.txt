set(PROJECT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}/include")

add_executable(main
    "main.cpp")
target_link_libraries(main PRIVATE raylib raygui)
target_include_directories(main PRIVATE ${PROJECT_INCLUDE_DIRECTORIES})
install(TARGETS main RUNTIME)
