# Initial setup
cmake_minimum_required(VERSION 3.28.1)
project(MP3PLAYER LANGUAGES CXX C VERSION 0.0.1)
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)
set(CMAKE_CXX_STANDARD 20)

# Default configuration values
set(DEFAULT_BUILD_TYPE "Release")
set(DEFAULT_ARCH "x86-64")

# Detect platform
string(COMPARE EQUAL "Linux"   ${CMAKE_SYSTEM_NAME} LINUX)
string(COMPARE EQUAL "Windows" ${CMAKE_SYSTEM_NAME} WINDOWS)
if(LINUX)
    set(OS_STRING "linux")
elseif(WINDOWS)
    set(OS_STRING "windows")
else()
    set(OS_STRING "Unknown")
endif()

# Detect compiler
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CLANG 1)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(CLANG 1)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(GCC 1)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(INTEL 1)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(MSVC 1)
endif()

# Set build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE ${DEFAULT_BUILD_TYPE})
endif()
string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)

# Set architecture to default if not set
if(NOT ARCH)
    set(ARCH ${DEFAULT_ARCH})
endif()

# Set bitness
if(${ARCH} STREQUAL "x86-64")
    set(BITNESS "64")
    set(BITNESS_FLAG "-m${BITNESS}")
elseif(${ARCH} STREQUAL "x86")
    set(BITNESS "32")
    set(BITNESS_FLAG "-m${BITNESS}")
elseif(${ARCH} STREQUAL "armv8")
    set(BITNESS "64")
    set(BITNESS_FLAG "")
else()
    set(BITNESS "32")
    set(ARCH "x86")
    set(BITNESS_FLAG "-m${BITNESS}")
    message("Unknown architecture selected, defaulting to x86")
endif()

if(MSVC)
    # User cannot specify bitness with MSVC, so set it to whatever the generator is.
    string(TOLOWER ${CMAKE_GENERATOR} GENERATOR)
    if(GENERATOR MATCHES ".*win64.*")
        set(BITNESS "64")
    else()
        set(BITNESS "32")
    endif()
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ox")

elseif(GCC OR CLANG)
    # Treat warning return-type as error to avoid undefined behaviour
    # when a non-void function does not return a value.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${BITNESS_FLAG} -std=c++11 -Werror=return-type")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wno-long-long")

elseif(INTEL)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${BITNESS_FLAG} -std=c++11")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
endif()


if(WINDOWS) 
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:100000000")
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
add_compile_definitions(_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING)
endif(WINDOWS) 

find_package(boost REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(ffmpeg REQUIRED)
find_package(freeglut REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glew REQUIRED)
find_package(imgui REQUIRED)
find_package(opengl REQUIRED)
find_package(opencv REQUIRED)
find_package(ZLIB REQUIRED)

set(CPACK_NSIS_CONTACT "rajiv.sithiravel@gmail.com")
	
set(IMFONTS_SRC_LIST
	assets/imfonts/RobotoBold.cpp
	assets/imfonts/RobotoItalic.cpp
	assets/imfonts/RobotoMonoBold.cpp
	assets/imfonts/RobotoMonoItalic.cpp
	assets/imfonts/RobotoMonoRegular.cpp
	assets/imfonts/RobotoRegular.cpp
)

set(IMPLOT_SRC_LIST
	assets/implot/implot.cpp
	assets/implot/implot_items.cpp
)

set(MP3PLAYER_SRC_LIST
	mp3/MP3Player.h
	mp3/MP3Visualization.h
	mp3/MP3Visualization.cpp
)

set(VISUALIZER_SRC_LIST
	assets/visualizer/VisualizationBase.cpp
)

set(BINDINGS_SRC_LIST
	bindings/imgui_impl_glfw.cpp
	bindings/imgui_impl_opengl3.cpp	
)

set(TEST_SRC_LIST
	test/ImGuiVis.cpp
	test/TestDemo.cpp	
)

add_executable(${PROJECT_NAME_LOWER} 
	${IMFONTS_SRC_LIST}
	${IMPLOT_SRC_LIST}
	${MP3PLAYER_SRC_LIST}
	${VISUALIZER_SRC_LIST}
	${BINDINGS_SRC_LIST}
	${TEST_SRC_LIST}
)
	
add_custom_command(TARGET ${PROJECT_NAME_LOWER}
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/visualizer/webfonts/fa-brands-400.ttf ${PROJECT_BINARY_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/visualizer/webfonts/fa-regular-400.ttf ${PROJECT_BINARY_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/visualizer/webfonts/fa-solid-900.ttf ${PROJECT_BINARY_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/assets/visualizer/webfonts/fa-v4compatibility.ttf ${PROJECT_BINARY_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/test/Oryza.mp3 ${PROJECT_BINARY_DIR}
)

if(WINDOWS) 				
target_compile_options(${PROJECT_NAME_LOWER} PRIVATE "/Od")   
endif(WINDOWS)

target_compile_definitions(${PROJECT_NAME_LOWER} PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)
target_include_directories(${PROJECT_NAME_LOWER} PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/assets/imfonts ${PROJECT_SOURCE_DIR}/assets/implot ${PROJECT_SOURCE_DIR}/mp3 ${PROJECT_SOURCE_DIR}/assets/visualizer ${PROJECT_SOURCE_DIR}/assets/visualizer/IconFontCppHeaders ${PROJECT_SOURCE_DIR}/bindings ${PROJECT_SOURCE_DIR}/test ) 
 
target_link_libraries(${PROJECT_NAME_LOWER} boost::boost Eigen3::Eigen ffmpeg::ffmpeg FreeGLUT::freeglut_static glfw GLEW::GLEW imgui::imgui opengl::opengl opencv::opencv ZLIB::ZLIB)

if(CMAKE_BUILD_TYPE STREQUAL DEBUG)
    message("Detected compiler and platform:")
	message("Clang:   ${CLANG}")
	message("GCC:     ${GCC}")
	message("Intel:   ${INTEL}")
	message("MSVC:    ${MSVC}")
	message("Linux:   ${LINUX}")
	message("Windows: ${WINDOWS}")
	message("OS X:    ${OS_X}")
endif()

message("Configuring ${PROJECT_NAME} version ${VERSION} in ${CMAKE_BUILD_TYPE} mode for ${ARCH} (${BITNESS} bit)")
message("Compiler flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}}")

