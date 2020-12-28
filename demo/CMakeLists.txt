cmake_minimum_required(VERSION 2.8)
#set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_BUILD_TYPE Release CACHE STRING "set build type to release")
#cmake . -DCMAKE_BUILD_TYPE=RELEASE

##############################################################################

set(acl_path ${CMAKE_CURRENT_SOURCE_DIR}/../acllib)
set(acl_include ${acl_path}/include)


set(aclredis_path ${CMAKE_CURRENT_SOURCE_DIR}/../aclredis)
set(aclredis_include ${aclredis_path}/include)

include_directories(
	${acl_include}
	${aclredis_include}
)

SET(LIB_PATH . ../acllib/lib ../aclredis/lib)


##############################################################################
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

if (CMAKE_BUILD_TYPE STREQUAL "RELEASE")
    add_definitions("-O3 -DNDEBUG")
    MESSAGE("build RELEASE")
else()
    add_definitions("-g")
    MESSAGE("build DEBUG")
endif()


LINK_DIRECTORIES(${LIB_PATH})
ADD_EXECUTABLE(demo test.cpp demo.cpp redistest.cpp) 
TARGET_LINK_LIBRARIES(demo   
					  dl 
                      aclredis  
                      acl 
                      pthread 
                      )

##############################################################################
