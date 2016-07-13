#
#      xGS 3D Low-level rendering API
#
#  Low-level 3D rendering wrapper API with multiple back-end support
#
#  (c) livingcreative, 2015 - 2016
#
#  https://github.com/livingcreative/xgs
#
#  opengl/OpenGL.cmake
#      cmake include file for OpenGL implementation build
#

# OpenGL implementation specific headers
set(HEADERS
	${HEADERS}
	opengl/xGScontext.h
	opengl/xGSdatabuffer.h
	opengl/xGSframebuffer.h
	opengl/xGSgeometry.h
	opengl/xGSgeometrybuffer.h
	opengl/xGSGLutil.h
	opengl/xGSimpl.h
	opengl/xGSinput.h
	opengl/xGSparameters.h
	opengl/xGSstate.h
	opengl/xGStexture.h
)

# OpenGL implementation specific sources
set(SOURCES
	${SOURCES}
	opengl/xGSmain.cpp

	# xGSdatabuffer.cpp
	# xGSframebuffer.cpp
	# xGSgeometry.cpp
	# xGSgeometrybuffer.cpp
	# xGSGLutil.cpp
	# xGSimpl.cpp
	# xGSinput.cpp
	# xGSparameters.cpp
	# xGSstate.cpp
	# xGStexture.cpp
)

# OpenGL implementation additional include dirs
include_directories(opengl/..)
include_directories(opengl)

# OpenGL implementation libs for shared library
set(SHAREDLIB_LIBS
	xGSOpenGLLib
)

# Windows specific headers and sources
if (WIN32)
	include(opengl/win/windows.cmake)
	set(PLATFORMPROP WIN32)
endif ()

# OpenGL implementation target names
set(xGSStaticLib xGSOpenGLLib)
set(xGSSharedLib xGSOpenGL)
