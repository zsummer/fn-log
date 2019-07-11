cmake_minimum_required(VERSION 3.0)
project(fn-log)
enable_language(ASM)

#from github.com/zsummer

# plat : WIN32 APPLE UNIX    (UNIX contain UNIX like)

# CMAKE_CXX_COMPILER_ID:  GNU Intel Clang AppleClang MSVC 
# example IF (CMAKE_CXX_COMPILER_ID MATCHES "Clang") ENDIF()

# jump compiler works check
# if(WIN32)
#    set(CMAKE_C_COMPILER_WORKS TRUE)
#    set(CMAKE_CXX_COMPILER_WORKS TRUE)
# endif(WIN32)

# jump this project build when msvc 
# set_target_properties(${PROJECT_NAME} PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)

# set this project setup build when msvc 
# set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# show msvc folder
#  SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON) 

# use C++ 14
# set(CMAKE_CXX_FLAGS -std=c++14) 

# 
# CMAKE_SOURCE_DIR   cmake root dir 
# CMAKE_CURRENT_SOURCE_DIR current cmakelist.txt dir  
# EXECUTABLE_OUTPUT_PATH can set it change bin out dir
# CMAKE_MODULE_PATH can set it change module dir 
# PROJECT_NAME cur project name 

# include 
# include_directories  
# link_directories 
# link_libraries 

# 
# execute_process
if( NOT CMAKE_CONFIGURATION_TYPES )
    set( CMAKE_CONFIGURATION_TYPES Debug Release )
endif( NOT CMAKE_CONFIGURATION_TYPES )

set(CMAKE_BUILD_TYPE Debug CACHE STRING "cache debug release "  )
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CMAKE_CONFIGURATION_TYPES} )

#分组添加到IDE
MACRO(GROUP_SRC_BY_DIR FILES)
    FOREACH(PATH_NAME ${${FILES}})
        STRING(REGEX REPLACE ${CMAKE_CURRENT_SOURCE_DIR}/\(.*\) \\1 RELATIVE_PATH ${PATH_NAME})
        STRING(REGEX MATCH "(.+)/.*" HAS_GROUP ${RELATIVE_PATH})
        IF(HAS_GROUP)
                STRING(REGEX REPLACE "(.+)/.*" \\1 GROUP_NAME ${RELATIVE_PATH})
                STRING(REPLACE "/" "\\" GROUP_NAME ${GROUP_NAME})
                MESSAGE(${PATH_NAME} " into " ${GROUP_NAME})
                SOURCE_GROUP(${GROUP_NAME} FILES ${PATH_NAME})
        ELSE()
                SOURCE_GROUP("" FILES ${PATH_NAME})
        ENDIF()
    ENDFOREACH(PATH_NAME)
ENDMACRO(GROUP_SRC_BY_DIR)


FILE(GLOB_RECURSE SOURCES_H ${CMAKE_SOURCE_DIR}/src/include/*.h)
# 自动INCLUDE所有头文件的目录  (可能会影响到include的优先级)
# list(FILTER <list> <INCLUDE|EXCLUDE> REGEX <regular_expression>)  

set(SOURCES ${SOURCES_H} ${SOURCES_C})
GROUP_SRC_BY_DIR(SOURCES)


FOREACH(FILE_NAME ${SOURCES_H})
        STRING( REGEX REPLACE "[^/\\\\]+$" " " DIR_NAME ${FILE_NAME} )
        MESSAGE(${FILE_NAME})
	MESSAGE(${DIR_NAME})
        LIST(APPEND DIR_NAME_LIST ${DIR_NAME})
ENDFOREACH()

IF(DIR_NAME_LIST)
    LIST(REMOVE_DUPLICATES DIR_NAME_LIST)
ENDIF()

FOREACH(DIR_NAME ${DIR_NAME_LIST})
        MESSAGE("auto include " ${DIR_NAME} )
        INCLUDE_DIRECTORIES(${DIR_NAME})
ENDFOREACH()

#通用设置部分 包括启用分组 设置 启动项目  
if(WIN32)
    ADD_DEFINITIONS(-DWIN32 -W3)
    SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON) 
    #SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
else()
    SET(CMAKE_CXX_FLAGS -std=c++11)
    SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
    SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
    LINK_LIBRARIES(pthread m c)
endif()

#输出
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)


#自定义部分 

FILE(GLOB_RECURSE TEST_CPP ${CMAKE_SOURCE_DIR}/tests/*.cpp)
FOREACH (FILE_NAME ${TEST_CPP})
    STRING(REGEX REPLACE ".+/(.+)\\..*" "\\1" TEST_NAME ${FILE_NAME})
    ADD_EXECUTABLE(${TEST_NAME} ${FILE_NAME} ${SOURCES})
    MESSAGE("TEST " ${TEST_NAME} )
endforeach()



