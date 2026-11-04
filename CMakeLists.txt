cmake_minimum_required(VERSION 3.16)
project(ShopDSA VERSION 1.0 LANGUAGES CXX)

# ─── C++ Standard ────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ─── Output directories ───────────────────────────────────────────────────────
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# ─── Source files ─────────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    # Data structures (header-only, but listed for IDE visibility)
    # src/ds/array_ds.h
    # src/ds/stack_ds.h
    # src/ds/queue_ds.h
    # src/ds/tree_ds.h
    # src/ds/graph_ds.h
    # src/algorithms/hanoi.h
    # src/ecommerce/product.h
    # src/ecommerce/catalog.h
    # src/ecommerce/cart.h
    # src/ecommerce/orders.h
)

# ─── Executable ───────────────────────────────────────────────────────────────
add_executable(ShopDSA ${SOURCES})

# ─── Include directories ──────────────────────────────────────────────────────
target_include_directories(ShopDSA PRIVATE
    ${CMAKE_SOURCE_DIR}/include   # httplib.h
    ${CMAKE_SOURCE_DIR}/src       # so we can use #include "ds/..." etc.
)

# ─── Windows Socket Library ───────────────────────────────────────────────────
if(WIN32)
    target_link_libraries(ShopDSA PRIVATE ws2_32)

    # Suppress deprecation warnings for WinSock functions
    target_compile_definitions(ShopDSA PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _WINSOCK_DEPRECATED_NO_WARNINGS
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
endif()

# ─── Compiler-specific flags ──────────────────────────────────────────────────
if(MSVC)
    target_compile_options(ShopDSA PRIVATE /W3 /EHsc)
else()
    # GCC / Clang (MinGW on Windows)
    target_compile_options(ShopDSA PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -O2
    )
endif()

# ─── Copy web assets next to the binary ───────────────────────────────────────
add_custom_command(TARGET ShopDSA POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/web
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/web
    COMMENT "Copying web assets to bin/web/"
)

# ─── Print summary ────────────────────────────────────────────────────────────
message(STATUS "")
message(STATUS "=== ShopDSA Build Configuration ===")
message(STATUS "  Compiler  : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Standard  : C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Output    : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "====================================")
message(STATUS "")
