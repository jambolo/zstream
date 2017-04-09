cmake_minimum_required (VERSION 3.8)
project(zstream CXX)

option(BUILD_SHARED_LIBS "Build libraries as DLLs" FALSE)

find_package(Doxygen)
if(DOXYGEN_FOUND)
    add_custom_target(doxygen ALL
        ${DOXYGEN_EXECUTABLE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
        SOURCES Doxyfile
    )
endif(DOXYGEN_FOUND)

set(CMAKE_LIBRARY_PATH
     ${CMAKE_SOURCE_DIR}/../3rdParty/zlib/lib
)
set(CMAKE_INCLUDE_PATH
     ${CMAKE_SOURCE_DIR}/../3rdParty/zlib/include
)

find_library(ZLIB zlib)
find_library(ZLIBD zlibd)
find_path(ZLIB_INCLUDE_DIR zlib/zlib.h)

#set(zstream_VERSION_MAJOR 0)
#set(zstream_VERSION_MINOR 1)
#configure_file("${PROJECT_SOURCE_DIR}/Version.h.in" "${PROJECT_BINARY_DIR}/Version.h")

set(PUBLIC_INCLUDE_PATHS
    include/zstream
    ${ZLIB_INCLUDE_DIR}
)

set(SOURCES
    include/zstream/zfilebuf.h
    include/zstream/zfstream.h
    include/zstream/zmembuf.h
    include/zstream/zmstream.h

    zfilebuf.cpp
    zfstream.cpp
    zmembuf.cpp
    zmstream.cpp
)

add_library(zstream ${SOURCES})
target_compile_definitions(zstream
    PRIVATE
        -DNOMINMAX
        -DWIN32_LEAN_AND_MEAN
        -DVC_EXTRALEAN
        -D_CRT_SECURE_NO_WARNINGS
        -D_SECURE_SCL=0
        -D_SCL_SECURE_NO_WARNINGS
)
target_include_directories(zstream PUBLIC ${PUBLIC_INCLUDE_PATHS})
target_link_libraries(zstream 
    debug ${ZLIBD}
    optimized ${ZLIB})

#add_subdirectory(test)
