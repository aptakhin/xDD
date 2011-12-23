cmake_minimum_required (VERSION 2.6)
project(XDD)

include_directories("sources/include/")
include_directories(${CMAKE_BINARY_DIR})

IF(MSVC)
    add_definitions(-D_UNICODE -DUNICODE -DWIN32 -DQT_LARGEFILE_SUPPORT -DQT_DLL -DQT_GUI_LIB -DQT_CORE_LIB -DQT_HAVE_MMX -DQT_HAVE_3DNOW -DQT_HAVE_SSE -DQT_HAVE_MMXEXT -DQT_HAVE_SSE2 -DQT_THREAD_SUPPORT)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:wchar_t-")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /level='requireAdministrator' /SUBSYSTEM:WINDOWS")
ENDIF(MSVC)

find_package(Qt4 REQUIRED QtCore QtGui QtMain)
include(${QT_USE_FILE})

message ("${CMAKE_SYSTEM_NAME}")

include("sources/CmakeSources.txt")

QT4_WRAP_CPP(XDD_MOCCED_FILES ${XDD_MOC_FILES})
QT4_WRAP_UI(XDD_UICED_FILES ${XDD_UI_FILES})

add_executable(xdd ${XDD_HEADER_FILES} ${XDD_SOURCE_FILES} ${XDD_MOCCED_FILES} ${XDD_UICED_FILES})
target_link_libraries(xdd ${QT_LIBRARIES})

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    message("Copying dlls and pdbs from ${QT_LIBRARY_DIR} to ${CMAKE_BINARY_DIR}")
    install(DIRECTORY ${QT_LIBRARY_DIR}/ DESTINATION ${CMAKE_BINARY_DIR} FILES_MATCHING PATTERN "*.dll")
endif()
