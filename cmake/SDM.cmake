# ensure the directory exists
if( NOT IS_DIRECTORY ${SDM_DIR} )
	message( FATAL_ERROR "Could not load SDM, directory doesn't exist. ${SDM_DIR}" )
	return()
endif()

# grab the sources
file( GLOB SDM_SOURCES ${SDM_DIR}/src/*.cpp )

# create target
add_library( SDM STATIC ${SDM_SOURCES} )

# add target include
target_link_libraries( SDM PUBLIC ${OpenCV_LIB_DIR} )
target_include_directories( SDM PUBLIC ${SDM_DIR}/include ${OpenCV_INCLUDE_DIRS} ${EIGEN3_INCLUDE_DIR} )