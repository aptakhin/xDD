cmake_minimum_required (VERSION 2.8.8)

project(XDD)

include_directories("sources/include/")
include_directories(${CMAKE_BINARY_DIR}) # For MOCced source files

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} $ENV{QTDIR})
 
add_definitions(-DNOMINMAX) # hate, hate, hate!

if (MSVC)

	add_definitions(-D_UNICODE -DUNICODE -DWIN32 -DQT_LARGEFILE_SUPPORT -DQT_DLL -DQT_GUI_LIB -DQT_CORE_LIB -DQT_HAVE_MMX -DQT_HAVE_3DNOW -DQT_HAVE_SSE -DQT_HAVE_MMXEXT -DQT_HAVE_SSE2 -DQT_THREAD_SUPPORT)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:wchar_t-")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /level='requireAdministrator'")

endif(MSVC)

find_package(Qt5Widgets)

include("sources/CMakeSources.txt")

qt5_wrap_cpp(XDD_MOCCED_FILES ${XDD_MOC_FILES})
qt5_wrap_ui(XDD_UICED_FILES ${XDD_UI_FILES})
qt5_add_resources(XDD_RES sources/res/all.qrc)

add_executable(xdd ${XDD_HEADER_FILES} ${XDD_SOURCE_FILES} ${XDD_MOCCED_FILES} ${XDD_UICED_FILES} ${XDD_RES})
target_link_libraries(xdd ${QT_LIBRARIES})

qt5_use_modules(xdd Widgets)

include(cotire)
cotire(xdd)
