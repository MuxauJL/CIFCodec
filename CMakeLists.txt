cmake_minimum_required(VERSION 3.19)

project(cif_codec LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

find_package(OpenCV REQUIRED imgcodecs imgproc)

file(GLOB SRC_FILES src/*.cpp)
file(GLOB HDR_FILES src/*.h)
file(GLOB UI_FILES src/*.ui)

file(GLOB UTILS_FILES utils/*.h)

set(PROJECT_SOURCES
        ${SRC_FILES}
        ${HDR_FILES}
        ${UI_FILES}
        ${UTILS_FILES}
)

set(executable_name CIFCodec)
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${executable_name} WIN32
        ${PROJECT_SOURCES}
    )
endif()

target_link_libraries(${executable_name} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    ${OpenCV_LIBS})
target_include_directories(${executable_name} PRIVATE
    ${OpenCV_INCLUDE_DIRS})
