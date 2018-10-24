cmake_minimum_required(VERSION 3.1)
project(acdc-daq)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

add_library(SuMo
        lib/SuMo.cpp
        lib/DAQinstruction.cpp
        lib/form_meta_data.cpp
        lib/GetAcdcPackets.cpp
        lib/GetSysPackets.cpp
        lib/oscilloscope.cpp
        lib/ScopePipe.cpp
        lib/dataIO.cpp
        lib/config.cpp)

add_library(myusb
        lib/usb/stdUSBl.cpp
        lib/usb/stdUSBl_Slave.cpp
        lib/usb/usbResetDevice.C)

add_library(calibration
        lib/calibration/loadPedandLin.cpp
        lib/calibration/makePedandLin.cpp)

add_library(yaml-cpp SHARED IMPORTED)
set_target_properties(yaml-cpp PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/yaml-cpp/libyaml-cpp.a)

target_include_directories(SuMo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(myusb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(calibration PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(myusb PRIVATE usb)
target_link_libraries(SuMo PRIVATE z)
target_link_libraries(SuMo PRIVATE m)
target_link_libraries(SuMo PRIVATE myusb)
target_link_libraries(SuMo PRIVATE calibration)
target_link_libraries(SuMo PRIVATE yaml-cpp)


add_executable(Automated_selfTrig src/Automated_SelfTrig.cpp)
add_executable(Automated_selfTrig_Cutoffs src/Automated_SelfTrig_Cutoffs.cpp)
add_executable(Automation src/Automation.cpp)
add_executable(ledEn src/ledEn.cpp)
add_executable(calEn src/calEn.cpp)
add_executable(dumpData src/dumpData.cpp)
add_executable(logData src/logData.cpp)
add_executable(makeLUT src/makeLUT.cpp)
add_executable(oScope src/oScope.cpp)
add_executable(readACDC src/readACDC.cpp)
add_executable(readCC src/readCC.cpp)
add_executable(Reset src/Reset.cpp)
add_executable(resetDLL src/resetDll.cpp)
add_executable(setConfig src/setConfig.cpp)
add_executable(setPed src/setPed.cpp)
add_executable(setupLVDS src/setupLVDS.cpp)
add_executable(takePed src/takePed.cpp)
add_executable(toggle_led src/toggle_led.cpp)

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
