find_package( Doxygen REQUIRED )
if( NOT DOXYGEN_FOUND )
	message( FATAL_ERROR "Doxygen is needed to build the documentation." )
	return()
endif()

set( doxyfile_in ${ROOT_DIR}/doc/Doxyfile.in )
set( doxyfile ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile )

configure_file( ${doxyfile_in} ${doxyfile} @ONLY )
add_custom_target( doc
	COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc
	COMMENT	"Generating API documentation with Doxygen..."
	VERBATIM
)

# install rules when install is executed,
# it will copy ${CMAKE_CURRENT_BINARY_DIR}/html to ${CMAKE_INSTALL_PREFIX}/doc}
# install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION doc)