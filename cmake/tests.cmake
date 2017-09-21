include( CMakeParseArguments )

function( add_test ARG_NAME )
	# Parse arguments
	cmake_parse_arguments( ARG "" "" "DIRECTORIES" ${ARGN} )

	# Get all source files
	list( APPEND ARG_DIRECTORIES "${ROOT_DIR}/tests/${ARG_NAME}" )
	set( SOURCES "" )
	foreach( DIR ${ARG_DIRECTORIES} )
		file( GLOB GLOB_SOURCES ${DIR}/*.cpp ${DIR}/*.hpp ${DIR}/*.h )
		list( APPEND SOURCES ${GLOB_SOURCES} )
	endforeach()
	add_executable( test-${ARG_NAME} ${SOURCES} )
	target_include_directories( test-${ARG_NAME} PRIVATE ${ROOT_DIR}/tests/include ${ARG_DIRECTORIES} )

	# configure_debugging( test-${ARG_NAME} WORKING_DIR ${...} )
	# Custom target as BUILD_ALL tests at once
	if( BUILD_TESTS )
		add_dependencies( tests test-${ARG_NAME} )
	endif()
	set_target_properties( test-${ARG_NAME} PROPERTIES FOLDER "tests" )
endfunction()


if( BUILD_TESTS )
	add_custom_target( tests )
	set_target_properties( tests PROPERTIES FOLDER "tests" )
endif()


set(
	TESTS
	xmath
	)

foreach( TEST ${TESTS} )
	add_test( 
		${TEST} 
		DIRECTORIES 
		${SDM_DIR}/include
		)
endforeach()