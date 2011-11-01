
option( ROINT_BUILD_TESTS "build tests" OFF )
if( ROINT_BUILD_TESTS )
remove_definitions( -DROINT_INTERNAL )
set( TESTS
	test_act
	test_gat
	test_gnd
	test_rgz
	test_spr
	test_text
	)
set( AUX_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/test.rgz"
	)

# tests
foreach( _NAME IN ITEMS ${TESTS} )
	add_executable( ${_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/${_NAME}.c" )
	target_link_libraries( ${_NAME} roint )
	add_dependencies( ${_NAME} roint )
endforeach()

# install
install( TARGETS ${TESTS}
	RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
	)
install( FILES ${AUX_FILES}
	DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
	)
endif( ROINT_BUILD_TESTS )