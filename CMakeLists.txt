# (C) Copyright 2020- ECMWF.
# (C) Copyright 2024- Meteo-France.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation
# nor does it submit to any jurisdiction.

#Changed : 3.18 instead of 3.12, from ectrans CMake configuration
cmake_minimum_required( VERSION 3.18 FATAL_ERROR )
find_package( ecbuild 3.4 REQUIRED HINTS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../ecbuild )

project( fiat LANGUAGES Fortran C CXX )
include( fiat_macros )
ecbuild_enable_fortran( REQUIRED NO_MODULE_DIRECTORY )

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

#Added : from ectrans CMake configuration
if( ${CMAKE_VERSION} VERSION_LESS "3.25" )
  if ( ECTRANS_ENABLE_ACC OR (NOT DEFINED ECTRANS_ENABLE_ACC AND ENABLE_ACC) )
    # See https://gitlab.kitware.com/cmake/cmake/-/issues/23691, fixed in CMake 3.25
    # (TL;DR: FindOpenACC sets OpenACC_<LANG>_FOUND correctly but does not set
    #  OpenACC_FOUND unless all three C, CXX, and Fortran have been found - even if
    #  only one language has been requested via COMPONENTS)
    find_package( OpenACC COMPONENTS Fortran )
    if( OpenACC_Fortran_FOUND )
      set( OpenACC_FOUND ON )
    endif()
  endif()
endif()

### Options

ecbuild_add_option( FEATURE OMP
                    DESCRIPTION "Support for OpenMP shared memory parallelism"
                    REQUIRED_PACKAGES "OpenMP COMPONENTS Fortran" )

ecbuild_add_option( FEATURE MPI
                    DESCRIPTION "Support for MPI distributed parallelism"
                    REQUIRED_PACKAGES "MPI COMPONENTS Fortran" )

ecbuild_add_option( FEATURE DR_HOOK_PAPI
                    DEFAULT OFF
                    DESCRIPTION "Support for HW counters in DR_HOOK via PAPI"
                    REQUIRED_PACKAGES "PAPI")

ecbuild_find_package( fckit QUIET )
ecbuild_add_option( FEATURE FCKIT
                    DESCRIPTION "Support for fckit"
                    CONDITION fckit_FOUND AND fckit_HAVE_ECKIT )

ecbuild_add_option( FEATURE DR_HOOK_MULTI_PRECISION_HANDLES
                    DESCRIPTION "[DEPRECATED] Support single precision handles for DR_HOOK"
                    DEFAULT OFF )

ecbuild_add_option( FEATURE DOUBLE_PRECISION
                    DEFAULT ON
                    DESCRIPTION "Support for Double Precision" )

ecbuild_add_option( FEATURE SINGLE_PRECISION
                    DEFAULT ON
                    DESCRIPTION "Support for Single Precision" )

ecbuild_add_option( FEATURE DUMMY_MPI_HEADER
                    DEFAULT ON
                    DESCRIPTION "[DANGEROUS] Install a dummy MPI header" )

# Two MPL interface styles
# - DEPRECATED F77 based interface, relies on mpif.h, can't take non-contiguous arrays
# - MPI_F08 based interface, relies on mpi_f08 module
# NB : both interfaces can be compiled without MPI, and then link to mpi_serial library
set( MPL_F77_DEPRECATED_DEFAULT OFF )
if( HAVE_MPI AND NOT MPI_Fortran_HAVE_F08_MODULE )
    ecbuild_warn( "MPI Fortran present, but no MPI-F08 support found, so building with FEATURE MPL_F77_DEPRECATED DEFAULT ON. "
                  "Please check your MPI installation for MPI-F08 support." )
    set( MPL_F77_DEPRECATED_DEFAULT ON )
endif()
ecbuild_add_option( FEATURE MPL_F77_DEPRECATED
                    DESCRIPTION "[DEPRECATED] Compile deprecated 'mpif.h'-based MPL instead of MPI_F08 version"
                    REQUIRED_PACKAGES "MPI COMPONENTS Fortran"
                    DEFAULT ${MPL_F77_DEPRECATED_DEFAULT} )

ecbuild_add_option( FEATURE MPL_CHECK_CONTIG
                    DESCRIPTION "Enable runtime check of contiguity of array arguments passed to MPL routines"
                    DEFAULT OFF )

ecbuild_add_option( FEATURE WARNINGS
                    DEFAULT ON
                    DESCRIPTION "Add warnings to compiler" )

#added : from ectrans CMake configuration
ecbuild_add_option( FEATURE ACC
                    DEFAULT OFF
                    DESCRIPTION "Support for using GPUs with OpenACC"
                    REQUIRED_PACKAGES "OpenACC COMPONENTS Fortran" )
ecbuild_add_option( FEATURE OMPGPU
                    DEFAULT OFF
                    DESCRIPTION "Support for using GPUs with OpenMP"
                    REQUIRED_PACKAGES "OpenMP COMPONENTS Fortran" )

if(CMAKE_C_COMPILER_ID STREQUAL "PGI" OR CMAKE_C_COMPILER_ID STREQUAL "NVHPC" )
    set (DEFAULT_DR_HOOK_NVTX ON)
else()
    set (DEFAULT_DR_HOOK_NVTX OFF)
endif()
ecbuild_add_option( FEATURE DR_HOOK_NVTX
                    DEFAULT ${DEFAULT_DR_HOOK_NVTX}
                    DESCRIPTION "Support for NVTX in DR_HOOK"
                    REQUIRED_PACKAGES NVTX )

set (DEFAULT_DR_HOOK_ROCTX OFF)
ecbuild_add_option( FEATURE DR_HOOK_ROCTX
                    DEFAULT ${DEFAULT_DR_HOOK_ROCTX}
                    DESCRIPTION "Support for ROCTX in DR_HOOK"
                    REQUIRED_PACKAGES ROCTX )

ecbuild_find_package( NAME Realtime QUIET )

#Added : from ectrans CMake configuration
# Search for available GPU runtimes, searching for CUDA first and, if not found,
# attempt to find HIP
if( ECTRANS_ENABLE_GPU OR (NOT DEFINED ECTRANS_ENABLE_GPU AND ENABLE_GPU))
  set(HAVE_CUDA 0)
  set(HAVE_HIP 0)
  ectrans_find_cuda() # sets "HAVE_CUDA"
  if( NOT HAVE_CUDA )
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24")
        ectrans_find_hip() # sets "HAVE_HIP"
    else()
        message("Cannot enable for HIP language, require CMake >= 3.24")
    endif()
  endif()
endif()

#Added : from ectrans CMake configuration
ecbuild_add_option( FEATURE GPU
                    DEFAULT OFF
                    DESCRIPTION "Compile GPU version of ectrans (Requires OpenACC or sufficient OpenMP offloading support)"
                    CONDITION (HAVE_HIP OR HAVE_CUDA) AND (HAVE_ACC OR HAVE_OMPGPU) )

#Added : from ectrans CMake configuration
if( HAVE_GPU )
  if( HAVE_ACC )
    set( GPU_OFFLOAD "ACC" )
  elseif( HAVE_OMPGPU )
    set( GPU_OFFLOAD "OMP" )
  else()
    ecbuild_error("Could not enable GPU as OMP or ACC were not enabled")
  endif()
endif()

## find fypp
ecbuild_find_package( fckit )
find_program( FYPP fypp )

if( fckit_FOUND AND fckit_HAVE_FCKIT_VENV )
  set( FYPP ${FCKIT_VENV_EXE} -m fypp )
elseif( fckit_FOUND )
  # This is only needed for building in environments with python3 older than 3.8
  list( APPEND _fckit_fypp_path "${FYPP}" )
  list( LENGTH _fckit_fypp_path _list_length )
  MATH( EXPR _last_entry "${_list_length} - 1" )
  list( GET _fckit_fypp_path ${_last_entry} FYPP )
elseif( FYPP MATCHES FYPP-NOTFOUND )
  set(FYPP ${fypp_SOURCE_DIR}/bin/fypp)
  ecbuild_info("fypp downloaded to: ${FYPP}")
endif()


if( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(_whole_archive "-Wl,-force_load")
  set(_no_whole_archive "")
else()
  if("${CMAKE_Fortran_COMPILER_ID}" MATCHES "NAG")
    set(_whole_archive "-Wl,-Wl,,--whole-archive")
    set(_no_whole_archive "-Wl,-Wl,,--no-whole-archive")
  else()
    set(_whole_archive "-Wl,--whole-archive")
    set(_no_whole_archive "-Wl,--no-whole-archive")
  endif()
endif()

### Sources
# define MPI_SERIAL_LIBRARIES before including src folder, for use in fiat library declaration
set(MPI_SERIAL_LIBRARIES ${_whole_archive} mpi_serial ${_no_whole_archive})

include( fiat_compiler_warnings )
add_subdirectory(src)
add_subdirectory(share)

### Tests

add_subdirectory(tests)

### Export

if (fiat_HAVE_MPL_F77_DEPRECATED)
set (fiat_HAVE_MPL_F08 0)
else()
set (fiat_HAVE_MPL_F08 1)
endif()

set( fiat_REQUIRES_PRIVATE_DEPENDENCIES FALSE )
get_target_property( target_build_type fiat TYPE )
if( target_build_type STREQUAL STATIC_LIBRARY OR TARGET fiat-static )
  set( fiat_REQUIRES_PRIVATE_DEPENDENCIES TRUE )
  # Make find_package modules available to downstream packages
  foreach( find_module
    FindNVTX.cmake
    FindROCTX.cmake
    FindPAPI.cmake )
    if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${find_module}")
      execute_process( COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${find_module}" "${CMAKE_CURRENT_BINARY_DIR}/${find_module}" )
    endif()
    install( FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${find_module}" DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/fiat" )
  endforeach()
endif()

ecbuild_install_project( NAME fiat )

ecbuild_print_summary()

