set( CMAKE_LEGACY_CYGWIN_WIN32 0 )
cmake_minimum_required( VERSION 2.8.3 )
if( NOT DEFINED ROINT_NO_PROJECT )
	project( roint C )
endif( NOT DEFINED ROINT_NO_PROJECT )
enable_language( C )
if( CYGWIN )
	unset( WIN32 )
endif( CYGWIN )


# 
# zlib (for roint)
# 
message( STATUS "Looking for zlib" )
find_package( ZLIB )
if( ZLIB_FOUND )
	set( HAVE_zlib ON  CACHE INTERNAL "" )
	message( STATUS "Looking for zlib - found" )
else( ZLIB_FOUND )
	message( STATUS "Looking for zlib - not found" )
	unset( HAVE_zlib CACHE )
endif( ZLIB_FOUND )


# 
# roint
# 
if( HAVE_zlib )
	option( BUILD_ROINT "build roint library" ON )
	set( BUILD_ROINT_TYPE "STATIC" CACHE STRING "build type: SHARED (dll/so) or STATIC (lib/a)" )
	set_property( CACHE BUILD_ROINT_TYPE  PROPERTY STRINGS "SHARED" "STATIC" )
endif( HAVE_zlib )
if( BUILD_ROINT )
message( STATUS "Creating target roint" )
message( STATUS "Library type: ${BUILD_ROINT_TYPE}" )

set( ROINT_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/cp949.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/des.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/grf.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/internal.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/memloader.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/roint.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/rostrings.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/rsm.h"
	)
set( ROINT_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/des.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/grf.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/memloader.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/roint.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/rostrings.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/rsm.c"
	)

set( LIBRARIES ${LINK_LIBRARIES}  )
set( INCLUDE_DIRS ${INCLUDE_DIRECTORIES} ${ZLIB_INCLUDE_DIR} )
set( DEFINITIONS "${COMPILE_DEFINITIONS}" )
set( SOURCE_FILES ${ROINT_HEADERS} ${ROINT_SOURCES} )
if( BUILD_ROINT_TYPE STREQUAL SHARED )
	set( DEFINITIONS "${DEFINITIONS} -DROINT_DLL" CACHE INTERNAL "" )
endif( BUILD_ROINT_TYPE STREQUAL SHARED )
include_directories( ${INCLUDE_DIRS} )
add_library( roint ${BUILD_ROINT_TYPE} ${SOURCE_FILES} )
target_link_libraries( roint ${LIBRARIES} )
set_target_properties( roint PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )

set( HAVE_roint ON  CACHE INTERNAL "" )
message( STATUS "Creating target roint - done" )
else( BUILD_ROINT )
message( STATUS "Skipping target roint (requires zlib)" )
unset( HAVE_roint CACHE )
if( NOT DEFINED ROINT_NO_PROJECT )
	message( FATAL_ERROR "nothing to build" )
endif( NOT DEFINED ROINT_NO_PROJECT )
endif( BUILD_ROINT )
