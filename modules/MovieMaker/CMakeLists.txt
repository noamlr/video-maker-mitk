cmake_minimum_required(VERSION 3.17)

project(MovieMaker LANGUAGES CXX)

# Qt compilations system variables.
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON) # enable linker to qt headers
set (CMAKE_AUTOUIC ON) #
set (CMAKE_AUTORCC ON) # enable link to qrc

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE   ON)

if(NOT DEFINED MITK_SUPERBUILD_DIR)
	set(MITK_SUPERBUILD_DIR "$ENV{MITK_BUILD_PATH}")
	# set(MITK_SUPERBUILD_DIR "/home/noa/MITK-build")
endif()

if(NOT DEFINED QT_PATH)
		# set(QT_PATH "/home/guilherme/Qt/5.12.9")
		# set(QT_PATH "/home/noa/Qt5.12.8/5.12.8")
		set(QT_DIR "$ENV{QT_ROOT_PATH}")

endif()

set(CMAKE_PREFIX_PATH
		"${MITK_SUPERBUILD_DIR}/MITK-build/"
		"${MITK_SUPERBUILD_DIR}/ep/src/VTK/CMake/"
		"${QT_PATH}/gcc_64/lib/cmake/Qt5/"
		)

find_package(MITK CONFIG REQUIRED)
find_package(OpenMP REQUIRED)
find_package(Qt5 COMPONENTS Widgets Charts REQUIRED)


set(cpp_files
		QmitkMovieMakerView.cpp
		QmitkAnimationItem.cpp
		QmitkAnimationItemDelegate.cpp
		QmitkAnimationWidget.cpp
		QmitkOrbitAnimationItem.cpp
		QmitkOrbitAnimationWidget.cpp
#		QmitkScreenshotMaker.cpp
		QmitkSliceAnimationItem.cpp
		QmitkSliceAnimationWidget.cpp
		QmitkTimeSliceAnimationItem.cpp
		QmitkTimeSliceAnimationWidget.cpp
		)

set(header_files
		QmitkMovieMakerView.h
		QmitkAnimationItem.h
		QmitkAnimationItemDelegate.h
		QmitkAnimationWidget.h
		QmitkOrbitAnimationItem.h
		QmitkOrbitAnimationWidget.h
#		QmitkScreenshotMaker.h
		QmitkSliceAnimationItem.h
		QmitkSliceAnimationWidget.h
		QmitkTimeSliceAnimationItem.h
		QmitkTimeSliceAnimationWidget.h
		)

set(ui_files
		QmitkMovieMakerView.ui
		QmitkOrbitAnimationWidget.ui
		QmitkScreenshotMakerControls.ui
		QmitkSliceAnimationWidget.ui
		QmitkTimeSliceAnimationWidget.ui
		)

set(qrc_files
		moviemaker.qrc
		)

add_library(${PROJECT_NAME} STATIC ${cpp_files} ${header_files} ${ui_files} ${qrc_files})

target_compile_definitions(${PROJECT_NAME} PUBLIC cxx_std_14)

# Required on Unix OS family to be able to be linked into shared libraries.
#set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(${PROJECT_NAME} PRIVATE
		MitkCore
		MitkQtWidgetsExt
		Qt5::Widgets
		)
