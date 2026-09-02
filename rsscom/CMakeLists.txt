##############################################
#                                            #
#            RssCom library          #
#				 @Emovis 2019                #
#                                            #
##############################################

cmake_minimum_required (VERSION 4.3)

include(${CMAKE_SOURCE_DIR}/cmake/copy_runtime_dlls.cmake)

# Defines version number, company name, company copyright to be used as version file info
file(READ "version" ver)
string(REGEX MATCH "MAJOR_VERSION ([0-9]*)" _ ${ver})
set(RSSCOM_MAJOR_VERSION  ${CMAKE_MATCH_1})

string(REGEX MATCH "MINOR_VERSION ([0-9]*)" _ ${ver})
set(RSSCOM_MINOR_VERSION ${CMAKE_MATCH_1})

string(REGEX MATCH "PATCH_VERSION ([0-9]*)" _ ${ver})
set(RSSCOM_PATCH_VERSION ${CMAKE_MATCH_1})
set(RSSCOM_VERSION ${RSSCOM_MAJOR_VERSION}.${RSSCOM_MINOR_VERSION}.${RSSCOM_PATCH_VERSION} )
set(RSSCOM_API_VERSION ${RSSCOM_MAJOR_VERSION}.${RSSCOM_MINOR_VERSION} )
message("version: ${RSSCOM_VERSION}")
set(PRODUCT_COMPANY_NAME "Emovis")
set(PRODUCT_COMPANY_COPYRIGHT "Copyright Emovis (C) 2019")

# Set project name with version (a project can be composed of various executables and/or libraries)
project(RSSCom VERSION ${RSSCOM_VERSION})

set(CMAKE_VERBOSE_MAKEFILE OFF)

# Option to specify whether to compile the library static or dynamic
option(static_build "Static build" OFF)
# Option to specify whether to build the unit tests or not
option(build_tests "Build tests" ON)
# Option to specify whether cmake will use conan.cmake cmake script or not
#	must be on for visual studio linux/windows build
#	must be off for CI/CD build
option(use_conan_cmake "Use Conan in Cmake" ON)


IF(static_build)
	set(CMAKE_LINK_RSSCOM_TYPE STATIC )
ELSE(static_build)
	set(CMAKE_LINK_RSSCOM_TYPE SHARED )
ENDIF(static_build)

set(CMAKE_CXX_STANDARD 17)

# Add RSSCOM_LIB_EXPORT definition in the preprocessor definitions
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DRSSCOM_LIB_EXPORT ")

# Find and include all source files 
file(GLOB_RECURSE RSSCOM_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/Sources/*.cpp" )
file(GLOB_RECURSE RSSCOM_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/Includes/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/Includes/*.h")

# add library RssCom with version info
if(WIN32)
	set(CMAKE_GENERATOR_PLATFORM x64)

	configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/version.rc.in
					${CMAKE_CURRENT_BINARY_DIR}/version.h
					@ONLY)
	
	configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/version.rc
					${CMAKE_CURRENT_BINARY_DIR}/version.rc
					@ONLY)
	
	add_library(${PROJECT_NAME} ${CMAKE_LINK_RSSCOM_TYPE} ${RSSCOM_HEADERS} ${RSSCOM_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/version.h ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
endif(WIN32)
if(UNIX)
	add_library(${PROJECT_NAME} ${CMAKE_LINK_RSSCOM_TYPE} ${RSSCOM_HEADERS} ${RSSCOM_SRCS})
endif(UNIX)

set_target_properties (${PROJECT_NAME} PROPERTIES VERSION ${RSSCOM_VERSION} SOVERSION ${RSSCOM_API_VERSION})
set_target_properties(${PROJECT_NAME}
    PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS TRUE
		RELEASE_POSTFIX "_r"            
		MINSIZEREL_POSTFIX "_r"
		RELWITHDEBINFO_POSTFIX "_r"
		
)

# Add Includes directory in project's includes definitions
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/Includes")

# Conan 2 generates CMake package files with CMakeDeps.
find_package(rdkafka CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE RdKafka::rdkafka++)

# Install
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_BINARY_DIR}/bin)
#install(DIRECTORY ${CONAN_LIB_DIRS_RDKAFKA}/ DESTINATION ${CMAKE_BINARY_DIR}/bin PATTERN "lib/*")

# If tests building is enabled add the unit tests sub-project
message("Building tests : ${build_tests}")
IF(build_tests)
	find_package(GTest CONFIG REQUIRED)
	# Enable CMake testing
	enable_testing()
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Tests/GTest)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Tests/Consumer)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Tests/Producer)
ENDIF(build_tests)

#set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR})
if(WIN32)

	# Copy all runtime dependencies to build directory
	#copy_runtime_dlls(${PROJECT_NAME} $<TARGET_FILE_DIR:${PROJECT_NAME}>)

#	# Copy all Qt runtime dependecies to build directory (including QtPlugins)
#	set(Qt5_BIN_DIR ${Qt5_DIR}/../../../bin)
#	add_custom_command(TARGET ${VBAVC_EXE} POST_BUILD
#    		COMMAND ${Qt5_BIN_DIR}/windeployqt.exe $<TARGET_FILE:${VBAVC_EXE}>
#	)
endif()

## Copy all config file to build directory
#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#  COMMAND ${CMAKE_COMMAND} -E copy
#  -t $<TARGET_FILE_DIR:${PROJECT_NAME}>
#  COMMAND_EXPAND_LISTS
#)

