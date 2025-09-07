cmake_minimum_required(VERSION 3.22)

# Get project name from the folder name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
project(${PROJECT_NAME} CXX)

set(CMAKE_CXX_STANDARD 20)

##########################################################################################
# MSVC runtime linking
##########################################################################################

# this is here to ensure the runtime is linked staticly for msvc
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
    endif()
endif()

##########################################################################################
# Platform Configuration
##########################################################################################

if(NOT DEFINED PLATFORM)
    set(PLATFORM "default")
endif()

set(LIBRARIES ${PROJECT_NAME})

# libraries array to be linked
# set(LIBRARIES ${PROJECT_NAME})

##########################################################################################
# Add dependencies with FetchContent
##########################################################################################

include(FetchContent)
function(add_git_dependency libName gitURL gitTag)
    FetchContent_Declare(${libName}
        GIT_REPOSITORY ${gitURL}
        GIT_TAG ${gitTag}
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )

    if(NOT ("${libName}" STREQUAL "libname"))
        FetchContent_MakeAvailable(${libName})
    endif()

endfunction()


##########################################################################################
# symbolicpp Setup
##########################################################################################

if(ON)
    set(dep symbolicpp)

    set(BUILD_TESTS NO
    CACHE BOOL "Build SymEngine tests")

    add_git_dependency(
        ${dep}
        https://github.com/BrettWilsonDev/SymbolicCpp-CMake.git
        main
    )

    FetchContent_GetProperties(${dep})

    list(APPEND LIBRARIES symbolicpp)
endif()

##########################################################################################
# Project executable setup
##########################################################################################

# Recursively get all source files
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")

# Recursively get all header files (.h and .hpp)
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_LIST_DIR}/src/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/vendor/*.hpp"
)

# Extract directories from header files
set(PROJECT_INCLUDE_DIRS "")
foreach(header ${HEADER_FILES})
    get_filename_component(dir ${header} DIRECTORY)
    list(APPEND PROJECT_INCLUDE_DIRS ${dir})
endforeach()

# Remove duplicates
list(REMOVE_DUPLICATES PROJECT_INCLUDE_DIRS)

# Declare the executable
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

# Include all header directories
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIRS})


##########################################################################################
# Compiler and linker flags
##########################################################################################

if(NOT ${PLATFORM} STREQUAL "Web")
    MESSAGE(STATUS "Running non-web platform configuration...")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
        # set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static -pthread")
        # target_link_libraries(${LIBRARIES} -static-libgcc -static-libstdc++ -static -lpthread -Wl,-Bstatic -lstdc++)
        target_link_libraries(${LIBRARIES})
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_link_libraries(${LIBRARIES} -static-libgcc -static-libstdc++ -static -pthread -Wl,-Bstatic -lstdc++)
        # target_link_libraries(${LIBRARIES})
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel" OR CMAKE_C_COMPILER_ID STREQUAL "Intel")
        target_link_libraries(${LIBRARIES} -static-intel -lpthread -Wl,-Bstatic -lstdc++)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        target_link_libraries(${LIBRARIES})

        if(OFF)
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
        endif()

        # dont forget to set  "cmake.configureSettings": { "CMAKE_BUILD_TYPE": "${buildType}" } in settings.json for vs code
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            message("-- console enabled")
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
        else()
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
        endif()
    else()
        target_link_libraries(${LIBRARIES})
    endif()
else()
    target_link_libraries(${LIBRARIES})
endif()

##########################################################################################
# Build flags and additional settings
##########################################################################################

add_definitions(-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})

if(CMAKE_BUILD_TYPE MATCHES "Debug")
else()
    if(WIN32)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
        endif()
        add_compile_definitions(NDEBUG)
    endif()
endif()

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

if(${PLATFORM} STREQUAL "Web")
    set(CMAKE_EXECUTABLE_SUFFIX ".js")
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".js")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
        -s ALLOW_MEMORY_GROWTH=1 \
        -s MODULARIZE=1 \
        -s EXPORT_NAME=Module \
        -s EXPORT_ES6=1 \
    ")

    target_link_libraries(${PROJECT_NAME} embind)
endif()