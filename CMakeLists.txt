cmake_minimum_required(VERSION 3.14)

if (WIN32)
    project(VideoPlayer LANGUAGES CXX)
elseif(UNIX)
    project(VideoPlayer)
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug, Release)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)


if (UNIX)

set (SOURCE_DIR         "${CMAKE_SOURCE_DIR}/src") 
set (INCLUDE_DIR        "${CMAKE_SOURCE_DIR}/include")
set (SOURCE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/src")

add_subdirectory(thirdlib/UNIX/glew/build/cmake)
add_subdirectory(thirdlib/UNIX/glfw)
add_subdirectory(thirdlib/UNIX/FFmpeg)
add_definitions(-DGL_SILENCE_DEPRECATION)

include_directories(${GLEW_INCLUDE_DIR})
include_directories(${INCLUDE_DIR}) 
include_directories(${SOURCE_DIR})

link_directories(${GLEW_LIB_DIR})

file(GLOB_RECURSE SRC_FILE
    "${SOURCE_DIR}/vendor/stb_image/*.cpp"
    "${SOURCE_DIR}/video_reader/*.cpp"
    "${SOURCE_DIR}/*.cpp"
)

file (GLOB_RECURSE INC_FILE
    "${SOURCE_DIR}/vendor/stb_image/*.h"
    "${SOURCE_DIR}/video_reader/*.h"
    "${INCLUDE_DIR}/*.h"
)

file (GLOB_RECURSE RES_FILE
    "${CMAKE_SOURCE_DIR}/res/shaders/*.shader"
)


else (WIN32)

set (SOURCE_DIR         "${CMAKE_SOURCE_DIR}/src") 
set (INCLUDE_DIR        "${CMAKE_SOURCE_DIR}/include")
set (SOURCE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/src")
set (GLEW_INCLUDE_DIR   "${CMAKE_SOURCE_DIR}/thirdlib/WIN/glew/include")
set (GLEW_LIB_DIR       "${CMAKE_SOURCE_DIR}/thirdlib/WIN/glew/lib")
set (GLFW_INCLUDE_DIR   "${CMAKE_SOURCE_DIR}/thirdlib/WIN/glfw/include")
set (GLFW_LIB_DIR       "${CMAKE_SOURCE_DIR}/thirdlib/WIN/glfw/lib")
set (FFMPEG_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/thirdlib/WIN/FFmpeg/include")
set (FFMPEG_LIB_DIR     "${CMAKE_SOURCE_DIR}/thirdlib/WIN/FFmpeg/lib")

include_directories(${GLFW_INCLUDE_DIR})
include_directories(${GLEW_INCLUDE_DIR})
include_directories(${FFMPEG_INCLUDE_DIR})
include_directories(${VENDOR_INCLUDE_DIR})
include_directories(${INCLUDE_DIR}) 
include_directories(${SOURCE_DIR})

link_directories(${GLEW_LIB_DIR})
link_directories(${GLFW_LIB_DIR})
link_directories(${FFMPEG_LIB_DIR})

file(GLOB_RECURSE SRC_FILE
    "${SOURCE_DIR}/vendor/stb_image/*.cpp"
    "${SOURCE_DIR}/video_reader/*.cpp"
    "${SOURCE_DIR}/*.cpp"
)

file (GLOB_RECURSE INC_FILE
    "${SOURCE_DIR}/vendor/stb_image/*.h"
    "${SOURCE_DIR}/video_reader/*.h"
    "${INCLUDE_DIR}/*.h"
)

file (GLOB_RECURSE RES_FILE
    "${CMAKE_SOURCE_DIR}/res/shaders/*.shader"
    "${CMAKE_SOURCE_DIR}/res/*.rc"
)

source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_FILE})
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${INC_FILE})
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${RES_FILE})

endif()

if(APPLE)
    list(APPEND EXTRA_LIBS
        "-framework OpenGL"
    )
elseif(UNIX)
    list(APPEND EXTRA_LIBS
        "-lGL -lGLU -lX11  -g"
    )
else(WIN32)
    list(APPEND EXTRA_LIBS
       opengl32 glfw3 glew32 User32 Gdi32 Shell32
)
endif()


# Add the executable

if(UNIX)
    add_executable(${PROJECT_NAME} ${SRC_FILE} ${INC_FILE} ${RES_FILE} ) 
elseif (WIN32) 
	add_executable(${PROJECT_NAME} ${SRC_FILE} ${INC_FILE} ${RES_FILE} ) 
endif()


# Add the target includes for MY_PROJECT 
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${SOURCE_DIR})


#===================== LINKING LIBRARIES =======================#
if (UNIX)
	target_link_libraries(${PROJECT_NAME} glfw  glew avformat swscale avdevice avformat avcodec avutil ${EXTRA_LIBS})
else (WIN32)
	target_link_libraries(${PROJECT_NAME} avformat swscale avdevice avformat avcodec avutil ${EXTRA_LIBS})
endif()
