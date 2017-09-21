# ensure the directory exists
if( NOT IS_DIRECTORY ${SDM_DIR} )
	message( FATAL_ERROR "Could not load SDM, directory doesn't exist. ${SDM_DIR}" )
	return()
endif()

# grab the sources
file( GLOB SDM_SOURCES ${SDM_DIR}/src/*.cpp ${SDM_DIR}/include/*.hpp ${SDM_DIR}/include/*.h )

# create target
add_executable( SDM ${SDM_SOURCES} )

# set variables
set(
	SDM_LIB_DEPENDENCES
	${OpenCV_LIBS}
	${Boost_LIBRARIES}
	)

set(
	SDM_INCLUDE_DIRS
	${SDM_DIR}/include
	${SUPERVISEDDESCENT_DIR}/../3rdparty/cereal-1.1.1/include
	${SUPERVISEDDESCENT_DIR}
	${OpenCV_INCLUDE_DIRS}
	${EIGEN3_INCLUDE_DIR}
	${Boost_INCLUDE_DIRS}
	)

# add target include
target_link_libraries( SDM PUBLIC ${SDM_LIB_DEPENDENCES} )
target_include_directories( SDM PUBLIC ${SDM_INCLUDE_DIRS} )