find_package(Threads REQUIRED)
include(ExternalProject)

#### GTEST and GMOCK ####

#ExternalProject_Add(
#        googletest
#        GIT_REPOSITORY https://github.com/google/googletest.git
#        UPDATE_COMMAND ""
#        INSTALL_COMMAND ""
#        LOG_DOWNLOAD ON
#        LOG_CONFIGURE ON
#        LOG_BUILD ON
#        PREFIX googletest)
#
#ExternalProject_Get_Property(googletest source_dir)
#include_directories(${source_dir}/googletest/include)
#include_directories(${source_dir}/googlemock/include)
#
#ExternalProject_Get_Property(googletest binary_dir)
#set(GTEST_LIBRARY_PATH ${binary_dir}/googlemock/gtest/${CMAKE_FIND_LIBRARY_PREFIXES}gtest.a)
#set(GTEST_LIBRARY gtest)
#add_library(${GTEST_LIBRARY} UNKNOWN IMPORTED)
#set_target_properties(${GTEST_LIBRARY} PROPERTIES
#        IMPORTED_LOCATION ${GTEST_LIBRARY_PATH}
#        IMPORTED_LINK_INTERFACE_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
#add_dependencies(${GTEST_LIBRARY} googletest)
#
#set(GTEST_MAIN_LIBRARY_PATH ${binary_dir}/googlemock/gtest/${CMAKE_FIND_LIBRARY_PREFIXES}gtest_main.a)
#set(GTEST_MAIN_LIBRARY gtest_main)
#add_library(${GTEST_MAIN_LIBRARY} UNKNOWN IMPORTED)
#set_target_properties(${GTEST_MAIN_LIBRARY} PROPERTIES
#        IMPORTED_LOCATION ${GTEST_MAIN_LIBRARY_PATH}
#        IMPORTED_LINK_INTERFACE_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
#add_dependencies(${GTEST_MAIN_LIBRARY} googletest)
#
#set(GMOCK_LIBRARY_PATH ${binary_dir}/googlemock/${CMAKE_FIND_LIBRARY_PREFIXES}gmock.a)
#set(GMOCK_LIBRARY gmock)
#add_library(${GMOCK_LIBRARY} UNKNOWN IMPORTED)
#set_target_properties(${GMOCK_LIBRARY} PROPERTIES
#        IMPORTED_LOCATION ${GMOCK_LIBRARY_PATH}
#        IMPORTED_LINK_INTERFACE_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
#add_dependencies(${GMOCK_LIBRARY} googletest)
#
#set(GMOCK_MAIN_LIBRARY_PATH ${binary_dir}/googlemock/${CMAKE_FIND_LIBRARY_PREFIXES}gmock_main.a)
#set(GMOCK_MAIN_LIBRARY gmock_main)
#add_library(${GMOCK_MAIN_LIBRARY} UNKNOWN IMPORTED)
#set_target_properties(${GMOCK_MAIN_LIBRARY} PROPERTIES
#        IMPORTED_LOCATION ${GMOCK_MAIN_LIBRARY_PATH}
#        IMPORTED_LINK_INTERFACE_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
#add_dependencies(${GMOCK_MAIN_LIBRARY} ${GTEST_LIBRARY})

#### YAML-CPP ####

ExternalProject_Add(yamlcpp
        GIT_REPOSITORY "https://github.com/jbeder/yaml-cpp"
        GIT_TAG "yaml-cpp-0.6.2"
        INSTALL_COMMAND ""
        TEST_COMMAND ""
        LOG_DOWNLOAD ON
        LOG_CONFIGURE ON
        LOG_BUILD ON
        PREFIX yamlcpp)

ExternalProject_Get_Property(yamlcpp source_dir)
ExternalProject_Get_Property(yamlcpp binary_dir)
include_directories(${source_dir}/include)

ExternalProject_Get_Property(yamlcpp binary_dir)
set(YAML_CPP_LIBRARY_PATH ${binary_dir}/libyaml-cpp.a)
set(YAML_CPP_LIBRARY yaml-cpp)
add_library(${YAML_CPP_LIBRARY} UNKNOWN IMPORTED)
set_target_properties(${YAML_CPP_LIBRARY} PROPERTIES
        IMPORTED_LOCATION ${YAML_CPP_LIBRARY_PATH})
add_dependencies(${YAML_CPP_LIBRARY} yamlcpp)