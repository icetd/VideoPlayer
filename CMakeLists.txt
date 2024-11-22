cmake_minimum_required(VERSION 3.10)
project(VideoPlayer)

add_subdirectory(thirdparty/glfw)
add_subdirectory(thirdparty/glm)

set (CMKAE_CXX_STANDARD 14)
set (CMAKE_BUILD_TYPE Release)
set (CMAKE_INCLUDE_CURRENT_DIR ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set (SOURCE_DIR  "${CMAKE_CURRENT_LIST_DIR}/src") 
set (INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/src")

set (GLFW_INC			"${CMAKE_CURRENT_LIST_DIR}/thirdparty/glfw/include")
set (GLAD_SRC			"${CMAKE_CURRENT_LIST_DIR}/thirdparty/glad/src")
set (GLAD_INC			"${CMAKE_CURRENT_LIST_DIR}/thirdparty/glad/include")
set (STB_IMAGE_INC		"${CMAKE_CURRENT_LIST_DIR}/thirdparty/stb_image")
set (GLM_INC			"${CMAKE_CURRENT_LIST_DIR}/thirdparty/glm")
set (IMGUI_SRC			"${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui")
set (FILEDIALOGS_SRC	"${CMAKE_CURRENT_LIST_DIR}/thirdparty/tinyfiledialogs")

if (WIN32)
	set (FFMPEG_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/FFmpeg/win/include")
	set (FFMPEG_LIB_DIR "${CMAKE_SOURCE_DIR}/thirdparty/FFmpeg/win/lib")
    set (JPEGTURBO_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/libjpeg-turbo/win/include")
    set (JPEGTURBO_LIB_DIR "${CMAKE_SOURCE_DIR}/thirdparty/libjpeg-turbo/win/lib")
elseif (UNIX)
    add_subdirectory(thirdparty/libjpeg-turbo/unix/libjpeg-turbo)
	add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/FFmpeg/unix)
endif()

set (VLD_INCLUDE_DIR    "${CMAKE_SOURCE_DIR}/thirdparty/VLD/include")
set (VLD_LIB_DIR        "${CMAKE_SOURCE_DIR}/thirdparty/VLD/lib")

link_directories(
		${FFMPEG_LIB_DIR}
		${VLD_LIB_DIR}
        ${JPEGTURBO_LIB_DIR}
)

include_directories(
	${INCLUDE_DIR}
	${INCLUDE_DIR}/Video
	${INCLUDE_DIR}/Utils
	${GLFW_INC}
	${GLAD_INC}
	${STB_IMAGE_INC}
	${GLM_INC}
	${IMGUI_SRC}
	${FFMPEG_INCLUDE_DIR}
    ${JPEGTURBO_INCLUDE_DIR}
	${VLD_INCLUDE_DIR}
) 

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

file(GLOB_RECURSE SRC_FILES
    "${SOURCE_DIR}/*.c*"
    "${SOURCE_DIR}/Video/*.c*"
    "${SOURCE_DIR}/Utils/*.c*"
	"${GLAD_SRC}/*c*"
	"${IMGUI_SRC}/*.c*"
)

file (GLOB_RECURSE INC_FILES
    "${INCLUDE_DIR}/*.h"
	"${IMGUI_SRC}/*.h"
	"${GLAD_INC}/*h"
)

file (GLOB_RECURSE RES_FILES
    "${CMAKE_SOURCE_DIR}/res/shaders/*.shader"
    "${CMAKE_SOURCE_DIR}/res/*.rc"
)


if (WIN32)
	source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${SRC_FILES})
	source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${INC_FILES})
	source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${RES_FILES})

elseif (UNIX)
#nothing to do
endif ()

if (WIN32)
	list(APPEND EXTRA_LIBS
		opengl32 glfw User32 Gdi32 Shell32
 	)
elseif (UNIX)
	list (APPEND EXTRA_LIBS
		GL GLU X11 glfw
	)
endif ()

add_executable(${PROJECT_NAME} ${SRC_FILES} ${INC_FILES} ${RES_FILES})

# Add the target includes for MY_PROJECT 
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${SOURCE_DIR})


#===================== LINKING LIBRARIES =======================#
target_link_libraries(${PROJECT_NAME} ${EXTRA_LIBS} swscale avformat avdevice  avcodec avutil jpeg)

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
if (WIN32)
file (GLOB FFMPEG_COPY_DLL "${CMAKE_SOURCE_DIR}/thirdparty/FFmpeg/win/dll/*" "${CMAKE_SOURCE_DIR}/thirdparty/libjpeg-turbo/win/dll/*")
file(COPY ${FFMPEG_COPY_DLL} DESTINATION ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_BUILD_TYPE})
file(COPY ${CMAKE_SOURCE_DIR}/config/imgui.ini DESTINATION ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_BUILD_TYPE})
file(COPY ${CMAKE_SOURCE_DIR}/res DESTINATION ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_BUILD_TYPE})
elseif (UNIX)
file(COPY ${CMAKE_SOURCE_DIR}/config/imgui.ini DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY ${CMAKE_SOURCE_DIR}/res DESTINATION ${EXECUTABLE_OUTPUT_PATH})
endif()
