cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

set (CMAKE_AUTOMOC ON)

project(videomaker)

set(MITK_SUPERBUILD_DIR "/home/noa/MITK-build")
set(QT_DIR "/home/noa")

set(CMAKE_PREFIX_PATH
		"${MITK_SUPERBUILD_DIR}/MITK-build"
		"${MITK_SUPERBUILD_DIR}/ep/src/VTK/CMake/"
		"${QT_DIR}/Qt5.12.8/5.12.8/gcc_64/lib/cmake/Qt5"
		)

find_package(MITK CONFIG REQUIRED)
find_package(OpenMP REQUIRED)

add_subdirectory(modules/MovieMaker)

set(cpp_files
  VideoMaker.cpp
  VideoMakerMain.cpp
	)

set(header_files
  VideoMaker.h
	)

add_executable(${PROJECT_NAME} ${cpp_files})
target_link_libraries(  ${PROJECT_NAME} PUBLIC
                        MitkCore  
						MitkQtWidgetsExt
						MovieMaker
						)


target_include_directories(${PROJECT_NAME} PUBLIC
		"${CMAKE_SOURCE_DIR}/modules/MovieMaker"
		)
