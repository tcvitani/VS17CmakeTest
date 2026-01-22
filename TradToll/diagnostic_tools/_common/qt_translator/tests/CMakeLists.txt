project(QtTranslatorTest LANGUAGES CXX)

set(TEST_EXE "${PROJECT_NAME}_vs17_x64")

#include(qt.cmake)

set(CMAKE_AUTOUIC ON)
set(AUTOGEN_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
 
#find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt6 #${QT_VERSION_MAJOR}
    COMPONENTS
        Core
        Gui
        Widgets
		Xml
		Core5Compat
#PATHS
#    $<$<PLATFORM_ID:Windows>:"C:/QT/6.10.1/lib/cmake"
) 

qt_standard_project_setup()


set(TEST_SOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sources)
set(TEST_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)
set(TEST_CONFIG
    ${CMAKE_CURRENT_SOURCE_DIR}/Others/qt_trans_gui_Languages.xml

)

 
configure_file("${TEST_RESOURCES_DIR}/version.rc.in" Resources.rc @ONLY)
 
qt_wrap_ui(TEST_UI_GEN_HEADERS
	${TEST_SOURCES_DIR}/FormMainTranslTest.ui

)

set(TEST_SOURCES
	${TEST_UI_GEN_HEADERS}
	${TEST_SOURCES_DIR}/main.cpp
	${TEST_SOURCES_DIR}/FormMainTranslTest.cpp
	${TEST_SOURCES_DIR}/FormMainTranslTest.h
)


qt_add_executable(${TEST_EXE} ${TEST_SOURCES}
)

target_include_directories(${TEST_EXE} PRIVATE
							${TEST_SOURCES_DIR}
							${TEST_RESOURCES_DIR}
							${EXPORT_HEADERS_DIR}
							${CMAKE_CURRENT_BINARY_DIR}
							"${CMAKE_SOURCE_DIR}/../../Commun_NT/declare/includes"
							"${CMAKE_SOURCE_DIR}/../../Voie_NT/commun/includes"
)

##-----------------------------------------------------------------------------------------------
										
##-----------------------------------------------------------------------------------------------
							
set_target_properties(${TEST_EXE}
    PROPERTIES
        WIN32_EXECUTABLE TRUE
		RELEASE_POSTFIX "_r"            
		MINSIZEREL_POSTFIX "_r"
		RELWITHDEBINFO_POSTFIX "_r"
)


target_link_libraries(${TEST_EXE}
    PUBLIC
		${PROJECT_LIB}
        Qt::Core
        Qt::Gui
        Qt::Widgets
		Qt::Xml
		Qt::Core5Compat
)

set(CMAKE_INSTALL_PREFIX  "C:/csr/exe")


add_custom_command(
    TARGET ${TEST_EXE} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -E copy
        -t ${CMAKE_INSTALL_PREFIX} 
        $<TARGET_FILE:${TEST_EXE}>
		${TEST_CONFIG}
    COMMAND_EXPAND_LISTS
)

#include(${CMAKE_SOURCE_DIR}/cmake/copy_runtime_dlls.cmake)

#copy_runtime_dlls(${TEST_EXE} $<TARGET_FILE_DIR:${TEST_EXE}>)