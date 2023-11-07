set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(raylib EXCLUDE_FROM_ALL)

add_library(raygui INTERFACE EXCLUDE_FROM_ALL)
target_link_libraries(raygui INTERFACE raylib)
target_include_directories(raygui SYSTEM INTERFACE "raygui/src")

