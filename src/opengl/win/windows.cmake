#
#      xGS 3D Low-level rendering API
#
#  Low-level 3D rendering wrapper API with multiple back-end support
#
#  (c) livingcreative, 2015 - 2016
#
#  https://github.com/livingcreative/xgs
#
#  opengl/win/windows.cmake
#      cmake include file for OpenGL Windows implementation
#

# windows platform build headers
set(HEADERS
	${HEADERS}

	opengl/win/glplatform.h
	opengl/win/xGScontextplatform.h
	opengl/win/xGSdefaultcontext.h
)

# additional include dirs for build
include_directories(opengl/win)

# On Windows OpenGL implementation depends on GLEW library
if (NOT GLEW)
	set(GLEW ${PROJECT_SOURCE_DIR}/../../glew)
endif ()
include_directories(${GLEW}/include)
link_directories(${GLEW}/lib)
add_definitions(-DGLEW_STATIC)
set(SHAREDLIB_LIBS
	${SHAREDLIB_LIBS}
	opengl32
	glew32s
)

# Windows shared library source files
set(SHARED_SOURCES
	opengl/win/xGSOpenGL.cpp
	xGS.def
)
