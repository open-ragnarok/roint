if( CMAKE_BUILD_TYPE STREQUAL "" )
	set( CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: None (CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE )
endif()

set( CMAKE_LEGACY_CYGWIN_WIN32 0 )
cmake_minimum_required( VERSION 2.8.3 )
project( roint C )
if( CYGWIN )
	unset( WIN32 )
endif( CYGWIN )


# variables
set( ROINT_LIBTYPE "SHARED" CACHE STRING "library type: SHARED (dll/so) or STATIC (lib/a)" )
set_property( CACHE ROINT_LIBTYPE  PROPERTY STRINGS "SHARED" "STATIC" )
if( MSVC )
	option( ROINT_USE_STATIC_RUNTIME_LIBRARY "link to the static runtime library" ON )
endif( MSVC )
file( GLOB ROINT_PUBLIC_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/roint/*.h"
	)
file( GLOB ROINT_PRIVATE_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/*.h"
	)
file( GLOB ROINT_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/*.c"
	)
include( CheckTypeSize REQUIRED )
CHECK_TYPE_SIZE( char SIZEOF_CHAR BUILTIN_TYPES_ONLY )
CHECK_TYPE_SIZE( short SIZEOF_SHORT BUILTIN_TYPES_ONLY )
CHECK_TYPE_SIZE( int SIZEOF_INT BUILTIN_TYPES_ONLY )
if( NOT (SIZEOF_CHAR EQUAL 1 AND SIZEOF_SHORT EQUAL 2 AND SIZEOF_INT EQUAL 4) )
	message( FATAL_ERROR "unexpected type sizes: SIZEOF_CHAR=${SIZEOF_CHAR} SIZEOF_SHORT=${SIZEOF_SHORT} SIZEOF_INT=${SIZEOF_INT}" )
endif()
find_package( ZLIB REQUIRED ) # dependency
if( ROINT_LIBTYPE STREQUAL "SHARED" )
	set( ROINT_DLL 1 )
else()
	set( ROINT_DLL )
endif()
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
	"${CMAKE_CURRENT_BINARY_DIR}/include/roint/config.h"
	)

# target
if( MSVC )
	add_definitions( -D_CRT_SECURE_NO_WARNINGS )
	add_definitions( -D_CRT_SECURE_NO_DEPRECATE )
	add_definitions( -D_CRT_NONSTDC_NO_DEPRECATE )
	foreach( _VAR IN ITEMS
		CMAKE_C_FLAGS  CMAKE_C_FLAGS_DEBUG  CMAKE_C_FLAGS_RELEASE
		CMAKE_C_FLAGS_MINSIZEREL  CMAKE_C_FLAGS_RELWITHDEBINFO
		)
		get_property( _STR CACHE ${_VAR} PROPERTY VALUE )
		if( ROINT_USE_STATIC_RUNTIME_LIBRARY )
			string( REPLACE "/MD" "/MT" _STR "${_STR}" )
		else( ROINT_USE_STATIC_RUNTIME_LIBRARY )
			string( REPLACE "/MT" "/MD" _STR "${_STR}" )
		endif( ROINT_USE_STATIC_RUNTIME_LIBRARY )
		set_property( CACHE ${_VAR} PROPERTY VALUE "${_STR}" )
	endforeach()
endif( MSVC )
add_definitions( -DROINT_INTERNAL )
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}/include" ${ZLIB_INCLUDE_DIRS} )
source_group( public FILES ${ROINT_PUBLIC_HEADERS} )
add_library( roint ${ROINT_LIBTYPE} ${ROINT_SOURCES} ${ROINT_PRIVATE_HEADERS} ${ROINT_PUBLIC_HEADERS} )
target_link_libraries( roint ${ZLIB_LIBRARIES} )


# install
install( TARGETS roint
	RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
	LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
	ARCHIVE DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
	)
install( DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/" "${CMAKE_CURRENT_BINARY_DIR}/include/"
	DESTINATION "${CMAKE_INSTALL_PREFIX}/include"
	)
