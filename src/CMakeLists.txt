set(CMAKE_SYSTEM_NAME Generic)

cmake_minimum_required(VERSION 3.16)

project(client)

include_directories(.)

set(SRCS
    Renderer/Common.c
    Ball.c
    Bitset.c
    Main.c
    Simulation.c
    SpatialHash.c
    Utilities.c
    Vector.c
    Window.c
)

if(USE_PROFILE)
    if(GENERATE)
        add_definitions(-fprofile-generate)
        set(LINKER_FLAGS "${LINKER_FLAGS} -fprofile-generate")
    else()
        add_definitions(-fprofile-use)
        set(LINKER_FLAGS "${LINKER_FLAGS} -fprofile-use")
    endif()
endif()

set(CMAKE_C_COMPILER "gcc")
if(DEBUG_BUILD)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -gdwarf-4")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -ffast-math -DNDEBUG -mtune=native -march=native")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math -DNDEBUG -mtune=native -march=native")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

if (WASM_BUILD)
    set(CMAKE_C_COMPILER "emcc")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --closure=1 -DWASM_BUILD -s INITIAL_MEMORY=33554432 -s NO_EXIT_RUNTIME=1 -s EXPORTED_FUNCTIONS=_malloc,_free,_tmp_renderer_main_loop,_main,_tmp_key_event,_tmp_mouse_event,_tmp_touch_event,_tmp_wheel_event,_tmp_paste_event")
    set(SRCS ${SRCS} Renderer/Wasm.c)
else()
    set(SRCS ${SRCS} Renderer/Native.cc)
    include_directories(~/Downloads/skia)
endif()

add_executable(client ${SRCS})
target_link_libraries(client m)
if (WASM_BUILD)
else()
    target_link_libraries(client skia glfw gcov)
endif()
