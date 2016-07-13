#
#      xGS 3D Low-level rendering API
#
#  Low-level 3D rendering wrapper API with multiple back-end support
#
#  (c) livingcreative, 2015 - 2016
#
#  https://github.com/livingcreative/xgs
#
#  CMakeLists.txt
#      xGS cmake project
#

cmake_minimum_required(VERSION 2.8)

project(xGS)

# relative path to public includes
set(INCLUDE_PATH ../include)

# relative path to executabels build destination
set(BINARY_OUT_PATH ${PROJECT_SOURCE_DIR}/../bin)

# relative path to library build destination
set(LIBRARY_OUT_PATH ${PROJECT_SOURCE_DIR}/../lib)

# relative path to libs
set(LIBS_PATH ${PROJECT_SOURCE_DIR}/../lib)


set(HEADERS
	# public include headers
	${INCLUDE_PATH}/xGS/xGS.h
	${INCLUDE_PATH}/xGS/IUnknown.h
	${INCLUDE_PATH}/xGS/InterfaceReference.h

	# private source common headers
	IUnknownImpl.h
	xGSobject.h
	xGSutil.h
)


set(SHARED_HEADERS
	# shared library needs only base xGS.h header
	${INCLUDE_PATH}/xGS/xGS.h
)


# OpenGL implementation
include(opengl/OpenGL.cmake)


# MSVC specific build
if (MSVC)
	include(msvc.cmake)
endif ()


# public include directories
include_directories(${INCLUDE_PATH})

# kcommon dependencies
if (NOT kcommon)
	set(kcommon ../../kcommon/include)
endif ()
include_directories(${kcommon})

# implementation additional link dirs
link_directories(${LIBS_PATH})


# Static library project (main project)
add_library(${xGSStaticLib} STATIC ${SOURCES} ${HEADERS})
set_target_properties(
	${xGSStaticLib}
	PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY ${LIBRARY_OUT_PATH}
)
if (LIB_PDB_DIR)
	set_target_properties(
		${xGSStaticLib}
		PROPERTIES
		PDB_OUTPUT_DIRECTORY ${LIB_PDB_DIR}
		PDB_NAME "${xGSStaticLib}_vc$(PlatformToolsetVersion).pdb"
	)
endif ()

# Dynamic library project
add_library(${xGSSharedLib} SHARED ${SHARED_SOURCES} ${SHARED_HEADERS})
add_dependencies(${xGSSharedLib} ${xGSStaticLib})
target_compile_definitions(
	${xGSSharedLib}
	PRIVATE GS_STATIC_LINK
)
set_target_properties(
	${xGSSharedLib}
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUT_PATH}
)
target_link_libraries(${xGSSharedLib} ${SHAREDLIB_LIBS})
if (MSVC)
	#Add def file to linker options for dynamic library
	set_target_properties(
		${xGSSharedLib}
		PROPERTIES
		LINK_FLAGS "/DEF:\"${PROJECT_SOURCE_DIR}/win/${xGSSharedLib}.def\""
	)
endif ()
