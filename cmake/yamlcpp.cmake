find_package(Threads REQUIRED)
include(ExternalProject)

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
set(YAML_CPP_INCLUDE_DIRS ${source_dir}/include)
set(YAML_CPP_LIBRARY yaml-cpp)
add_library(${YAML_CPP_LIBRARY} STATIC IMPORTED)
set_target_properties(${YAML_CPP_LIBRARY} PROPERTIES
        IMPORTED_LOCATION ${binary_dir}/libyaml-cpp.a)
add_dependencies(${YAML_CPP_LIBRARY} yamlcpp)
