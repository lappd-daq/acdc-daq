cmake_minimum_required(VERSION 3.1)
project(acdc-daq VERSION 2.0 DESCRIPTION "Software to run the PSEC electronics")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

add_library(SuMo
        src/SuMo.cpp
        src/DAQinstruction.cpp
        src/form_meta_data.cpp
        src/GetAcdcPackets.cpp
        src/GetSysPackets.cpp
        src/oscilloscope.cpp
        src/ScopePipe.cpp)

add_library(myusb
        src/usb/stdUSBl.cpp
        src/usb/stdUSBl_Slave.cpp
        src/usb/usbResetDevice.C)

add_library(calibration
        src/calibration/loadPedandLin.cpp
        src/calibration/makePedandLin.cpp)



target_include_directories(SuMo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(myusb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(calibration PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(myusb PUBLIC usb)
target_link_libraries(SuMo PRIVATE z)
target_link_libraries(SuMo PRIVATE m)
target_link_libraries(SuMo PRIVATE myusb)
target_link_libraries(SuMo PRIVATE calibration)


add_executable(ledEn src/functions/ledEn.cpp)
add_executable(calEn src/functions/calEn.cpp)
add_executable(dumpData src/functions/dumpData.cpp)
add_executable(makeLUT src/functions/makeLUT.cpp)
add_executable(oScope src/functions/oScope.cpp)
add_executable(readACDC src/functions/readACDC.cpp)
add_executable(readCC src/functions/readCC.cpp)
add_executable(Reset src/functions/Reset.cpp)
add_executable(resetDLL src/functions/resetDll.cpp)
add_executable(setPed src/functions/setPed.cpp)
add_executable(setupLVDS src/functions/setupLVDS.cpp)
add_executable(takePed src/functions/takePed.cpp)
add_executable(toggle_led src/functions/toggle_led.cpp)

target_link_libraries(ledEn PUBLIC SuMo)
target_link_libraries(calEn PUBLIC SuMo)
target_link_libraries(dumpData PUBLIC SuMo)
target_link_libraries(makeLUT PUBLIC SuMo)
target_link_libraries(oScope PUBLIC SuMo)
target_link_libraries(readACDC PUBLIC SuMo)
target_link_libraries(readCC PUBLIC SuMo)
target_link_libraries(Reset PUBLIC SuMo)
target_link_libraries(resetDLL PUBLIC SuMo)
target_link_libraries(setPed PUBLIC SuMo)
target_link_libraries(setupLVDS PUBLIC SuMo)
target_link_libraries(takePed PUBLIC SuMo)
target_link_libraries(toggle_led PUBLIC SuMo)
