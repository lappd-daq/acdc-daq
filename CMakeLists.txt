cmake_minimum_required(VERSION 3.1)
project(acdc-daq)
set(acdc-daq_VERSION_MAJOR 1)
set(acdc-daq VERSION_MINOR 2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_SOURCE_DIR "${CMAKE_SOURCE_DIR}/lib")
set(EXECUTABLE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")

include(ExternalProject)
ExternalProject_Add(project_yaml_cpp
        GIT_REPOSITORY "https://github.com/jbeder/yaml-cpp"
        GIT_TAG "yaml-cpp-0.6.2"
        INSTALL_COMMAND ""
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/yaml-cpp)
ExternalProject_Get_Property(project_yaml_cpp source_dir)
ExternalProject_Get_Property(project_yaml_cpp binary_dir)

add_library(yaml-cpp SHARED IMPORTED)
add_dependencies(yaml-cpp project_yaml_cpp)
set_property(TARGET yaml-cpp PROPERTY IMPORTED_LOCATION ${binary_dir}/libyaml-cpp.a)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${source_dir}/include)

add_library(SuMo
        ${LIBRARY_SOURCE_DIR}/SuMo.cpp
        ${LIBRARY_SOURCE_DIR}/DAQinstruction.cpp
        ${LIBRARY_SOURCE_DIR}/form_meta_data.cpp
        ${LIBRARY_SOURCE_DIR}/GetAcdcPackets.cpp
        ${LIBRARY_SOURCE_DIR}/GetSysPackets.cpp
        ${LIBRARY_SOURCE_DIR}/oscilloscope.cpp
        ${LIBRARY_SOURCE_DIR}/ScopePipe.cpp
        ${LIBRARY_SOURCE_DIR}/dataIO.cpp
        ${LIBRARY_SOURCE_DIR}/config.cpp)

add_library(myusb
        ${LIBRARY_SOURCE_DIR}/usb/stdUSBl.cpp
        ${LIBRARY_SOURCE_DIR}/usb/stdUSBl_Slave.cpp
        ${LIBRARY_SOURCE_DIR}/usb/usbResetDevice.C)

add_library(calibration
        ${LIBRARY_SOURCE_DIR}/calibration/loadPedandLin.cpp
        ${LIBRARY_SOURCE_DIR}/calibration/makePedandLin.cpp)


target_link_libraries(myusb PRIVATE usb)
target_link_libraries(SuMo z m myusb calibration yaml-cpp)



add_executable(Automated_selfTrig ${EXECUTABLE_SOURCE_DIR}/Automated_SelfTrig.cpp)
add_executable(Automated_selfTrig_Cutoffs ${EXECUTABLE_SOURCE_DIR}/Automated_SelfTrig_Cutoffs.cpp)
add_executable(Automation ${EXECUTABLE_SOURCE_DIR}/Automation.cpp)
add_executable(ledEn ${EXECUTABLE_SOURCE_DIR}/ledEn.cpp)
add_executable(calEn ${EXECUTABLE_SOURCE_DIR}/calEn.cpp)
add_executable(dumpData ${EXECUTABLE_SOURCE_DIR}/dumpData.cpp)
add_executable(logData ${EXECUTABLE_SOURCE_DIR}/logData.cpp)
add_executable(makeLUT ${EXECUTABLE_SOURCE_DIR}/makeLUT.cpp)
add_executable(oScope ${EXECUTABLE_SOURCE_DIR}/oScope.cpp)
add_executable(readACDC ${EXECUTABLE_SOURCE_DIR}/readACDC.cpp)
add_executable(readCC ${EXECUTABLE_SOURCE_DIR}/readCC.cpp)
add_executable(Reset ${EXECUTABLE_SOURCE_DIR}/Reset.cpp)
add_executable(resetDLL ${EXECUTABLE_SOURCE_DIR}/resetDll.cpp)
add_executable(setConfig ${EXECUTABLE_SOURCE_DIR}/setConfig.cpp)
add_executable(setPed ${EXECUTABLE_SOURCE_DIR}/setPed.cpp)
add_executable(setupLVDS ${EXECUTABLE_SOURCE_DIR}/setupLVDS.cpp)
add_executable(takePed ${EXECUTABLE_SOURCE_DIR}/takePed.cpp)
add_executable(toggle_led ${EXECUTABLE_SOURCE_DIR}/toggle_led.cpp)

target_link_libraries(Automated_selfTrig PUBLIC SuMo)
target_link_libraries(Automated_selfTrig_Cutoffs PUBLIC SuMo)
target_link_libraries(Automation PUBLIC SuMo)
target_link_libraries(ledEn PUBLIC SuMo)
target_link_libraries(calEn PUBLIC SuMo)
target_link_libraries(dumpData PUBLIC SuMo)
target_link_libraries(logData PUBLIC SuMo)
target_link_libraries(makeLUT PUBLIC SuMo)
target_link_libraries(oScope PUBLIC SuMo)
target_link_libraries(readACDC PUBLIC SuMo)
target_link_libraries(readCC PUBLIC SuMo)
target_link_libraries(Reset PUBLIC SuMo)
target_link_libraries(resetDLL PUBLIC SuMo)
target_link_libraries(setConfig PUBLIC SuMo)
target_link_libraries(setPed PUBLIC SuMo)
target_link_libraries(setupLVDS PUBLIC SuMo)
target_link_libraries(takePed PUBLIC SuMo)
target_link_libraries(toggle_led PUBLIC SuMo)
