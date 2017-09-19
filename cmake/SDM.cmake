# ensure the directory exists
if( NOT IS_DIRECTORY ${SDM_DIR} )
	message( FATAL_ERROR "Could not load SDM, directory doesn't exist. ${SDM_DIR}" )
	return()
endif()

# grab the sources
file( GLOB SDM_SOURCES ${SDM_DIR}/src/*.cpp ${SDM_DIR}/include/*.hpp ${SDM_DIR}/include/*.h )

# create target
add_executable( SDM ${SDM_SOURCES} )

# add target include
target_link_libraries( SDM PUBLIC ${OpenCV_LIBS} ${Boost_LIBRARIES} )
target_include_directories( SDM PUBLIC ${SDM_DIR}/include ${SUPERVISEDDESCENT_DIR} ${OpenCV_INCLUDE_DIRS} ${EIGEN3_INCLUDE_DIR} ${Boost_INCLUDE_DIRS} )