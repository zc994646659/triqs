# Consider ROOT env variables in find_package
if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

# Version file
configure_file(version.hpp.in version.hpp @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/version.hpp" DESTINATION "include/triqs/")

# ---------------------------------
#  Get the sources
# ---------------------------------

# all .cpp files to get the dependencies
file(GLOB_RECURSE ALLSOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.cxx)
list(REMOVE_ITEM ALLSOURCES arrays/python/numpy_extractor.cpp)

# ---------------------------------
#  Define and configure the triqs target 
# ---------------------------------

add_library(triqs ${ALLSOURCES} ${CMAKE_CURRENT_BINARY_DIR}/git_hash.cpp)
set_property(TARGET triqs PROPERTY VERSION ${TRIQS_VERSION_MAJOR}.${TRIQS_VERSION_MINOR})

# Include dirs
target_include_directories(triqs PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/c++>)
target_include_directories(triqs SYSTEM INTERFACE $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>)

# Compile Options
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++20" HAS_CPP20)
if(HAS_CPP20)
  target_compile_options(triqs PUBLIC -std=c++20)
else()
  target_compile_options(triqs PUBLIC -std=c++17 $<$<CXX_COMPILER_ID:GNU>:-fconcepts>)
endif()

# Disable warnings about comma expression in array subscript
target_compile_options(triqs PUBLIC
  $<$<CXX_COMPILER_ID:GNU>:-Wno-comma-subscript>
  $<$<CXX_COMPILER_ID:Clang>:-Wno-deprecated-comma-subscript>
  $<$<CXX_COMPILER_ID:Clang>:-Wno-unknown-warning-option>
  $<$<CXX_COMPILER_ID:AppleClang>:-Wno-deprecated-comma-subscript>
  $<$<CXX_COMPILER_ID:AppleClang>:-Wno-unknown-warning-option>
)

# Compile Definitions
option(CHECK_MEMORY "Turn on memory check" OFF)
target_compile_definitions(triqs
  PUBLIC
    $<$<CONFIG:Debug>:TRIQS_DEBUG>
    $<$<CONFIG:Debug>:TRIQS_ARRAYS_ENFORCE_BOUNDCHECK>
    $<$<BOOL:${CHECK_MEMORY}>:TRIQS_ARRAYS_DEBUG_COUNT_MEMORY>
    $<$<BOOL:${CHECK_MEMORY}>:TRIQS_ARRAYS_CHECK_WEAK_REFS>
)

# This choice should be up to the user..
if((${CMAKE_SYSTEM_NAME} MATCHES "Darwin") AND ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang"))
 target_compile_options(triqs PUBLIC -stdlib=libc++)
endif()

# enforce color diagnostics ?
# gcc : -fdiagnostics-color
# clang :  -fcolor-diagnostics 

# ---------------------------------
# Link Dependencies
# ---------------------------------

# RT on linux only
target_link_libraries(triqs PUBLIC $<$<PLATFORM_ID:Linux>:-lrt>)

# Itertools
target_link_libraries(triqs PUBLIC itertools::itertools_c)

# MPI Interface
target_link_libraries(triqs PUBLIC mpi::mpi_c)

# H5 Interface
target_link_libraries(triqs PUBLIC h5::h5_c)

# HDF5 (still used explicitly in many_body_operator)
target_link_libraries(triqs PRIVATE h5::hdf5)

## NDA
#target_link_libraries(triqs PUBLIC nda::nda_c)

## blas_lapack
#target_link_libraries(triqs PUBLIC nda::blas_lapack)

# ========= Blas / Lapack ==========

message(STATUS "-------- Lapack detection -------------")

find_package(LAPACK REQUIRED)

# Create an interface target
add_library(blas_lapack INTERFACE)
list(REMOVE_DUPLICATES LAPACK_LIBRARIES)
target_link_libraries(blas_lapack INTERFACE ${LAPACK_LIBRARIES})
target_compile_options(blas_lapack INTERFACE ${LAPACK_LINKER_FLAGS})

# Link against interface target and export
target_link_libraries(triqs PUBLIC blas_lapack)
install(TARGETS blas_lapack EXPORT triqs-dependencies)

# ---------------------------------
# Boost
# ---------------------------------

message(STATUS "-------- Boost detection -------------")

set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost 1.53 REQUIRED COMPONENTS)
message(STATUS "Boost include dir: ${Boost_INCLUDE_DIRS}")

add_library(boost INTERFACE)
target_include_directories(boost SYSTEM INTERFACE ${Boost_INCLUDE_DIRS})

target_link_libraries(triqs PUBLIC boost)
install(TARGETS boost EXPORT triqs-dependencies)

# ---------------------------------
# GMP
# ---------------------------------

message(STATUS "-------- GMP detection -------------")
find_package(GMP)

target_link_libraries(triqs PUBLIC gmp)
install(TARGETS gmp EXPORT triqs-dependencies)

# ---------------------------------
# FFTW
# ---------------------------------

message(STATUS "-------- FFTW detection -------------")
find_package(FFTW)

target_link_libraries(triqs PUBLIC fftw)
install(TARGETS fftw EXPORT triqs-dependencies)

# ---------------------------------
# pthread
# ---------------------------------

# VERY OLD.
# remove the possible horrible pthread bug on os X !!( on gcc, old, before clang... is it really needed now ???)
# check for clang compiler ?? on gcc, os X snow leopard, it MUST be set
# since _REENTRANT is mysteriously set and this leads to random stalling of the code....
target_compile_options(triqs PUBLIC $<$<PLATFORM_ID:Darwin>:-pthread>)

# ---------------------------------
# max_align_t bug detection
# ---------------------------------

message(STATUS "-------- Misc -------------")

# should be move to config.h ?
try_compile(_max_align_t_is_correct ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake/max_align_t_test.cpp COMPILE_DEFINITIONS -std=c++11)
if(NOT _max_align_t_is_correct)
  message(STATUS "Adding workaround for max_align_t bug (in clang normally)")
  target_compile_definitions(triqs PUBLIC TRIQS_ADD_MAX_ALIGN_T_WORKAROUND)
else()
  message(STATUS "Checked max_align_t. No workaround needed")
endif()

# ---------------------------------
# Dynamic Analyzer Checks
# ---------------------------------

# Sanitizer Options already defined in NDA

if(ASAN)
  target_link_libraries(triqs PUBLIC $<BUILD_INTERFACE:asan>)
endif()
if(UBSAN)
  target_link_libraries(triqs PUBLIC $<BUILD_INTERFACE:ubsan>)
endif()

if(MSAN)
  if(NOT TARGET msan)
    add_library(msan INTERFACE)
    target_compile_options(msan INTERFACE -fsanitize=memory -fno-omit-frame-pointer -fsanitize-recover=all -fsanitize-memory-track-origins=2 -O1 -ggdb3)
    target_link_libraries(msan INTERFACE "-fsanitize=memory -fno-omit-frame-pointer -fsanitize-recover=all -fsanitize-memory-track-origins=2")
  endif()
  target_link_libraries(triqs PUBLIC $<BUILD_INTERFACE:msan>)
endif()

# ---------------------------------
# Build the triqs_py library
# ---------------------------------

add_library(triqs_py arrays/python/numpy_extractor.cpp)
set_property(TARGET triqs_py PROPERTY VERSION ${TRIQS_VERSION_MAJOR}.${TRIQS_VERSION_MINOR})
# C.f. https://gitlab.kitware.com/cmake/cmake/-/issues/15415
# for use of BUILD_INTERFACE
target_link_libraries(triqs_py PUBLIC triqs cpp2py::python_and_numpy $<BUILD_INTERFACE:cpp2py::cpp2py>)

# ---------------------------------
# Generate Compiler Wrapper
# ---------------------------------

# Get the compiler and linktime flags of triqs and python_and_numpy
include(${PROJECT_SOURCE_DIR}/cmake/extract_flags.cmake)
extract_flags(triqs)
extract_flags(cpp2py::python_and_numpy)

# Generate compiler wrapper
configure_file(triqs++.in triqs++ @ONLY)

# ---------------------------------
# Install 
# ---------------------------------

# Install the library in lib and prepare an exported cmake file to reimport it
install(TARGETS triqs triqs_py DESTINATION lib EXPORT triqs-targets)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/triqs++ PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE DESTINATION bin)
install(EXPORT triqs-dependencies NAMESPACE triqs:: DESTINATION lib/cmake/triqs)
install(EXPORT triqs-targets DESTINATION lib/cmake/triqs)

# Install headers 
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION include FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h" PATTERN "*.hxx" PATTERN "*.mako.*" EXCLUDE)
