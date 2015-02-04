cmake_minimum_required(VERSION 2.8.5)
project(ppam_mpi)

include("${CMAKE_SOURCE_DIR}/../shared_CMakeLists.txt")

#ppam executable (serialize, cluster, classif)

#cds
find_path(CDS_INCLUDE_PATH "cds/cds.h")
find_library(CDS_LIBRARY cds)
include_directories(${CDS_INCLUDE_PATH})

#GSL: standard location
find_path(GSL_INCLUDE_PATH "gsl/gsl_linalg.h")
include_directories(${GSL_INCLUDE_PATH})
find_library(GSL_LIBRARY gsl)
find_library(GSLCBLAS_LIBRARY gslcblas)
set(GSL_LIBRARIES ${GSL_LIBRARY} ${GSLCBLAS_LIBRARY})

#MPI: standard location
find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

#LibXml2: standard location
find_package(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})

file(GLOB_RECURSE ppam_src "${CMAKE_SOURCE_DIR}/*.c")
add_executable(ppam.exe ${ppam_src})
add_library(ppam SHARED ${ppam_src})
target_link_libraries(ppam.exe m cds ${MPI_LIBRARIES} ${LIBXML2_LIBRARIES} ${GSL_LIBRARIES} ${CDS_LIBRARY})
target_link_libraries(ppam m cds ${MPI_LIBRARIES} ${LIBXML2_LIBRARIES} ${GSL_LIBRARIES} ${CDS_LIBRARY})

if (MPI_COMPILE_FLAGS)
	set_target_properties(ppam.exe PROPERTIES
		COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif()
if (MPI_LINK_FLAGS)
	set_target_properties(ppam.exe PROPERTIES
		LINK_FLAGS "${MPI_LINK_FLAGS}")
endif()
