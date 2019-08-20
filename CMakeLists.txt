option(BUILD_STEP "" ON)
option(DEBUG "" ON)

# kept because we change these often (Linux compiler flags)
if(UNIX)

	# sets internal libs paths and flags
	include(${CMAKE_SOURCE_DIR}/cmake/cu.cmake)
	initsubmodules()

	cmake_minimum_required(VERSION 3.9)
	project(copper)

	if(DEBUG)
		# -mavx 
		set(OPT_FLAGS "-g -O0 -fno-omit-frame-pointer -fstack-protector-strong")

	endif()

	set(
		FLAGS_CXX
		"-msse4.1 -mfma -fdiagnostics-absolute-paths -fvisibility=hidden -march=x86-64 -fno-fast-math -fno-exceptions -fno-rtti -std=c++14 ${OPT_FLAGS} ${STRICT_FLAGS} ${CLANG_DEBUG}"
		)


	set(
		FLAGS_C
		"-msse4.1 -mfma -fdiagnostics-absolute-paths -fvisibility=hidden -march=x86-64 -fno-fast-math -fno-exceptions -fno-rtti -std=c11 ${OPT_FLAGS} ${STRICT_FLAGS} ${CLANG_DEBUG}"
		)

else(UNIX)

	cmake_minimum_required(VERSION 3.9)
	project(copper)

	# sets internal libs paths and flags
	include(${CMAKE_SOURCE_DIR}/cmake/cu.cmake)
	initsubmodules()

endif(UNIX)

set(CMAKE_CXX_FLAGS ${FLAGS_CXX})
set(CMAKE_C_FLAGS ${FLAGS_C})

#this adds LTO support
include(CheckIPOSupported)
check_ipo_supported(RESULT LTO_SUPPORTED)

if(LTO_SUPPORTED)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
	message(STATUS " LTO is enabled")
endif()

# Output Directory Variables
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Include Directories
include_directories("${CMAKE_SOURCE_DIR}/include"
	"${CMAKE_SOURCE_DIR}/src/")

# source files. all unity builds
file(GLOB RAYTRACER src/main.c)



option(IS_INDEPENDENT "" OFF)
set(CU_STD_DIR "${CMAKE_SOURCE_DIR}/Cu_std")
set(MODE_INCLUDE "${CMAKE_SOURCE_DIR}/")
include(${CMAKE_SOURCE_DIR}/Cu_std/RefCMakeLists.txt)

if(UNIX)
	target_compile_options(cu_std BEFORE PRIVATE -fPIC)
endif(UNIX)


# for these we do not want to have stdc++
# disable linking to stdc++ (Only works on linux)
set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")
set(CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES "")

set(CMAKE_C_IMPLICIT_LINK_LIBRARIES "")
set(CMAKE_C_IMPLICIT_LINK_DIRECTORIES "")


add_executable(raytracer ${RAYTRACER})

set_target_properties(raytracer PROPERTIES LINKER_LANGUAGE C)

