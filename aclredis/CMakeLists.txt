cmake_minimum_required(VERSION 2.8)
#set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_BUILD_TYPE Release CACHE STRING "set build type to release")
#cmake . -DCMAKE_BUILD_TYPE=RELEASE

#string(TOUPPER ${CMAKE_SYSTEM_NAME} CMAKE_SYSTEM_NAME)
if(CMAKE_SYSTEM_NAME MATCHES "Android")
	add_definitions("-DANDROID")
	set(UNIX_OS true)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(UNIX_OS true)
elseif(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	set(UNIX_OS true)
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
	set(UNIX_OS true)
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
	set(WIN_OS true)
else()
	message(FATAL_ERROR "unknown CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}")
endif()

##############################################################################

set(acl_path ${CMAKE_CURRENT_SOURCE_DIR}/../acllib)
set(acl_include ${acl_path}/include)


include_directories(
	${acl_include}
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/mw
)

#if (${WIN_OS})
#	set(iconv_include ${CMAKE_CURRENT_SOURCE_DIR}/src/stdlib/internal)
#	include_directories(iconv_include)
#endif()

set(src ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(sources
	${src}
	${src}/connpool
	${src}/ipc
	${src}/redis
	${src}/stdlib
	${src}/stream lib_src
	${CMAKE_CURRENT_SOURCE_DIR}/include/mw
)

foreach(iter ${sources})
	aux_source_directory(${iter} lib_src)
endforeach()

##############################################################################

if (${UNIX_OS})
	add_definitions(
		"-W"
		"-O2"
		"-fPIC"
		"-Wall"
		"-Werror"
		"-Wshadow"
		"-Wformat"
		"-Wpointer-arith"
		"-D_REENTRANT"
		"-D_USE_FAST_MACRO"
		"-Wno-long-long"
		"-Wuninitialized"
		"-D_POSIX_PTHREAD_SEMANTICS"
		"-DHAS_MYSQL_DLL"
		"-DHAS_PGSQL_DLL"
		"-DHAS_SQLITE_DLL"
		"-DHAS_POLARSSL_DLL"
		"-Wno-unused-parameter"
		"-fexceptions"
        "-std=c++11"
		)

	if (!CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
		add_definitions("-Wno-invalid-source-encoding"
			"-Wno-unused-private-field"
			"-Wno-unused-const-variable"
			)
	endif()

	if (CMAKE_BUILD_TYPE STREQUAL "RELEASE")
		add_definitions("-O3 -DNDEBUG")
        MESSAGE("build RELEASE")
	else()
		add_definitions("-g")
        MESSAGE("build DEBUG")
	endif()
elseif (${WIN_OS})
	add_definitions(
		"-Ycacl_stdafx.hpp"
		"-DACL_WRITEABLE_CHECK"
		"-DACL_PREPARE_COMPILE"
		"-DHAS_ZLIB_DLL"
		"-DHAS_MYSQL_DLL"
		"-DHAS_SQLITE_DLL"
		"-DHAS_PGSQL_DLL"
		"-DHAS_POLARSSL_DLL"
		"-DUSE_WIN_ICONV")
	aux_source_directory(${src}/stdlib/internal lib_src)
endif()

if (CMAKE_SYSTEM_NAME MATCHES "Android")
	# set(CMAKE_SHARED_LINKER_FLAGS "-shared -lz")
	set(CMAKE_SHARED_LINKER_FLAGS "-lz")
	add_definitions("-Wno-invalid-source-encoding")
else()
	set(lib_output_path ${PROJECT_BINARY_DIR}/lib)

	set(LIBRARY_OUTPUT_PATH ${lib_output_path})
	add_library(aclredis_static STATIC ${lib_src})
	SET_TARGET_PROPERTIES(aclredis_static PROPERTIES OUTPUT_NAME "aclredis")

	if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
		set(CMAKE_SHARED_LINKER_FLAGS
			"-shared -Wl,-rpath,. -L${lib_output_path} -lprotocol -lacl -liconv -lz -lpthread")
	elseif (${UNIX_OS})
		set(CMAKE_SHARED_LINKER_FLAGS
			"-shared -Wl,-rpath,. -L${lib_output_path} -lprotocol -lacl -lz -lpthread")
	elseif (${WIN_OS})
		set(CMAKE_SHARED_LINKER_FLAGS "/DYNAMICBASE ws2_32.lib")
	endif()

	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${lib_output_path})
	link_directories(${lib_output_path})
endif()

if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	#add_library(aclredis SHARED ${lib_src})
    add_library(aclredis ${lib_src})
	target_link_libraries(aclredis  acl)
endif()

##############################################################################
