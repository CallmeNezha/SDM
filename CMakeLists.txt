project( SDM )
cmake_minimum_required( VERSION 3.5 )
set( SDM_VERSION_MAJOR 0 )
set( SDM_VERSION_MINOR 1 )
set( SDM_VERSION_PATCH 0 )

set_property( GLOBAL PROPERTY USE_FOLDERS ON )

# c++11 support
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )

# set environment variables
set( ROOT_DIR 		${CMAKE_SOURCE_DIR} )
message(STATUS "ROOT_DIR: ${ROOT_DIR}/SDM")
set( THIRDPATY_DIR  ${ROOT_DIR}/thirdparty )

# set output directory wrt file type
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/arch" )

# set relative path
set( SDM_DIR "${ROOT_DIR}/SDM" CACHE PATH "SDM directory." )
set( DOC_DIR "${ROOT_DIR}/doc" CACHE PATH "Document directory Doxygen." )
set( CEREAL_INCLUDE_DIR "${THIRDPATY_DIR}/cereal-1.1.1/include" CACHE PATH "cereal-1.1.1 directory." )


# options
option( BUILD_TESTS 	"Builds the unit test." 	ON )
option( BUILD_EXAMPLES  "Build the examples."  		ON )
option( BUILD_APPS		"Build the face alignment." ON )
option( BUILD_DOCUMENT  "Build doxygen documents."  ON )

message( STATUS "Options:" )
message( STATUS "BUILD_TESTS: ${BUILD_TESTS}" )
message( STATUS "BUILD_EXAMPLES: ${BUILD_EXAMPLES}" )
message( STATUS "BUILD_APPS: ${BUILD_APPS}" )
message( STATUS "BUILD_DOCUMENT: ${BUILD_DOCUMENT}" )

# find dependencies:
find_package( OpenCV 3.2.0 REQUIRED )
message( STATUS "OpenCV include dir found at ${OpenCV_INCLUDE_DIRS}" )
message( STATUS "OpenCV lib dir found at ${OpenCV_LIB_DIR}" )

set( CMAKE_MODULE_PATH cmake )
find_package( Eigen3 REQUIRED )
message( STATUS "Eigen3 found: ${EIGEN3_FOUND}" ) 
message( STATUS "Eigen3 include dir found at ${EIGEN3_INCLUDE_DIR}" )
message( STATUS "Eigen3 version ${EIGEN3_VERSION}" )

include( cmake/SDM.cmake )
if( BUILD_DOCUMENT )
	include( cmake/doc.cmake )
endif()
