cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

project(thread_pool
	DESCRIPTION "Library that provide a simple thread pool"
        LANGUAGES CXX)

add_library(thread_pool thread_pool.cpp)

option(BUILD_TESTS "Build unit tests" OFF)
if (BUILD_TESTS)
	include(FetchContent)
	FetchContent_Declare(
		GoogleTest
		GIT_REPOSITORY https://github.com/google/googletest.git
		GIT_TAG v1.16.0
	)
	# For Windows: Prevent overriding the parent project's compiler/linker settings
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
	FetchContent_MakeAvailable(googletest)
	
	enable_testing()
	message(STATUS "Building tests...")
	add_executable(tests testing_thread_pool.cpp)
	target_link_libraries(tests GTest::gtest)
	target_link_libraries(tests thread_pool)
	include(GoogleTest)	
	
	gtest_discover_tests(tests)
endif()
