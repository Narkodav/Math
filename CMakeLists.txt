cmake_minimum_required(VERSION 3.20)
project(Math LANGUAGES CXX)

# =========================
# Source files for library
# =========================
file(GLOB_RECURSE SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Math/*.cpp
)

option(MATH_BUILD_SHARED "Build Math as a shared library" OFF)

# =========================
# Library target
# =========================
if(MATH_BUILD_SHARED)
    add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
else()
    add_library(${PROJECT_NAME} STATIC ${SRC_FILES})
endif()

add_library(Math::Math ALIAS ${PROJECT_NAME})

# C++20 and warnings
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# Preprocessor definition
target_compile_definitions(${PROJECT_NAME} PRIVATE MATH_EXPORTS)

# Include directories
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# =========================
# Test executable
# =========================
file(GLOB_RECURSE TEST_SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp
)

add_executable(MathTests ${TEST_SRC_FILES})

# Link the library
target_link_libraries(MathTests PRIVATE ${PROJECT_NAME})

# Match C++ standard and warnings
target_compile_features(MathTests PUBLIC cxx_std_20)
target_compile_options(MathTests PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

target_include_directories(MathTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Enable CTest
enable_testing()
add_test(NAME MathTests COMMAND MathTests)

# =========================
# Install rules
# =========================
install(TARGETS ${PROJECT_NAME}
    EXPORT MathTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include
)

install(EXPORT MathTargets
    FILE MathTargets.cmake
    NAMESPACE Math::
    DESTINATION lib/cmake/Math
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/MathConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/MathConfig.cmake
    INSTALL_DESTINATION lib/cmake/Math
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/MathConfig.cmake
    DESTINATION lib/cmake/Math
)